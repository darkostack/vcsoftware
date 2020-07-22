#include <assert.h>

#include "core/instance.hpp"
#include "core/thread.hpp"
#include "core/code_utils.hpp"

namespace mt {

Thread *Thread::Init(Instance &aInstance, char *aStack, int aStackSize,
                     char aPriority, int aFlags, mtThreadHandlerFunc aHandlerFunc,
                     void *aArg, const char *aName)
{
    if (aPriority >= MTOS_CONFIG_THREAD_PRIORITY_LEVELS) return NULL;

    int totalStackSize = aStackSize;

    /* aligned the stack on 16/32 bit boundary */
    uintptr_t misalignment = reinterpret_cast<uintptr_t>(aStack) % 8;

    if (misalignment)
    {
        misalignment = 8 - misalignment;
        aStack += misalignment;
        aStackSize -= misalignment;
    }

    /* make room for the Thread */
    aStackSize -= sizeof(Thread);

    /* round down the stacksize to multiple of Thread aligments (usually 16/32 bit) */
    aStackSize -= aStackSize % 8;

    if (aStackSize < 0)
    {
        // TODO: warning: stack size is to small
    }

    /* allocate thread control block (tcb) at the top of our stackspace */
    Thread *tcb = (Thread *)(aStack + aStackSize);

    if (aFlags & THREAD_FLAGS_CREATE_STACKMARKER)
    {
        /* assign each int of the stack the value of it's address, for test purposes */
        uintptr_t *stackmax = reinterpret_cast<uintptr_t *>(aStack + aStackSize);
        uintptr_t *stackp = reinterpret_cast<uintptr_t *>(aStack);

        while (stackp < stackmax)
        {
            *stackp = reinterpret_cast<uintptr_t>(stackp);
            stackp++;
        }
    }
    else
    {
        /* create stack guard */
        *(uintptr_t *)aStack = reinterpret_cast<uintptr_t>(aStack);
    }

    unsigned state = mtCpuIrqDisable();

    /* initialize instances for this thread */
#if MTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
    tcb->mInstance = static_cast<void *>(&aInstance);
#else
    (void)aInstance;
#endif

    mtKernelPid pid = KERNEL_PID_UNDEF;

    for (mtKernelPid i = KERNEL_PID_FIRST; i <= KERNEL_PID_LAST; ++i)
    {
        if (tcb->Get<ThreadScheduler>().GetThreadFromScheduler(i) == NULL)
        {
            pid = i;
            break;
        }
    }

    if (pid == KERNEL_PID_UNDEF)
    {
        mtCpuIrqRestore(state);
        return NULL;
    }

    tcb->Get<ThreadScheduler>().SetThreadToScheduler(tcb, pid);

    tcb->SetPid(pid);

    tcb->StackInit(aHandlerFunc, aArg, aStack, aStackSize);

    tcb->SetStackStart(aStack);

    tcb->SetStackSize(totalStackSize);

    tcb->SetName(aName);

    tcb->SetPriority(aPriority);

    tcb->SetStatus(THREAD_STATUS_STOPPED);

    tcb->InitRunqueueEntry();

    tcb->InitMsg();

    tcb->Get<ThreadScheduler>().IncrementNumOfThreadsInScheduler();

    if (aFlags & THREAD_FLAGS_CREATE_SLEEPING)
    {
        tcb->Get<ThreadScheduler>().SetThreadStatusAndUpdateRunqueue(tcb, THREAD_STATUS_SLEEPING);
    }
    else
    {
        tcb->Get<ThreadScheduler>().SetThreadStatusAndUpdateRunqueue(tcb, THREAD_STATUS_PENDING);

        if (!(aFlags & THREAD_FLAGS_CREATE_WOUT_YIELD))
        {
            mtCpuIrqRestore(state);

            tcb->Get<ThreadScheduler>().ContextSwitch(aPriority);

            return tcb;
        }
    }

    mtCpuIrqRestore(state);

    return tcb;
}

void Thread::StackInit(mtThreadHandlerFunc aFunction, void *aArg, void *aStackStart, int aStackSize)
{
    this->SetStackPointer(mtThreadArchStackInit(aFunction, aArg, aStackStart, aStackSize));
}

void Thread::AddToList(List *aList)
{
    assert(GetStatus() < THREAD_STATUS_RUNNING);

    uint8_t myPriority = GetPriority();

    List *myNode = static_cast<List *>(GetRunqueueEntry());

    while (aList->mNext)
    {
        Thread *threadOnList = Thread::GetThreadPointerFromItsListMember(static_cast<List *>(aList->mNext));

        if (threadOnList->GetPriority() > myPriority)
        {
            break;
        }

        aList = static_cast<List *>(aList->mNext);
    }

    myNode->mNext = aList->mNext;

    aList->mNext = myNode;
}

Thread *Thread::GetThreadPointerFromItsListMember(List *aList)
{
    mtListNode *list = static_cast<mtListNode *>(aList);

    mtThread *thread = mtCONTAINER_OF(list, mtThread, mRunqueueEntry);

    return static_cast<Thread *>(thread);
}

void Thread::InitMsgQueue(Msg *aArray, int aNum)
{
    mMsgArray = aArray;

    (static_cast<Cib *>(&mMsgQueue))->Init(aNum);
}

int Thread::HasMsgQueue(void)
{
    return mMsgArray != NULL;
}

int Thread::GetNumOfMsgInQueue(void)
{
    int queuedMsg = -1;

    if (HasMsgQueue())
    {
        queuedMsg = (static_cast<Cib *>(&mMsgQueue))->Avail();
    }

    return queuedMsg;
}

int Thread::IsPidValid(mtKernelPid aPid)
{
    return ((KERNEL_PID_FIRST <= aPid) && (aPid <= KERNEL_PID_LAST));
}

int Thread::QueuedMsg(Msg *aMsg)
{
    int index = (static_cast<Cib *>(&mMsgQueue))->Put();

    if (index < 0)
    {
        return 0;
    }

    Msg *dest = static_cast<Msg *>(&mMsgArray[index]);

    *dest = *aMsg;

    return 1;
}

void Thread::InitMsg(void)
{
    mWaitData = NULL;
    mMsgWaiters.mNext = NULL;
    (static_cast<Cib *>(&mMsgQueue))->Init(0);
    mMsgArray = NULL;
}

void ThreadScheduler::Run(void)
{
    DisableContextSwitchRequestFromIsr();

    Thread *currentThread = GetCurrentActiveThread();

    Thread *nextThread = GetNextThreadFromRunqueue();

    if (currentThread == nextThread) return;

    if (currentThread != NULL)
    {
        if (currentThread->GetStatus() == THREAD_STATUS_RUNNING)
        {
            currentThread->SetStatus(THREAD_STATUS_PENDING);
        }
    }

    nextThread->SetStatus(THREAD_STATUS_RUNNING);

    SetCurrentActiveThread(nextThread);

    SetCurrentActivePid(nextThread->GetPid());
}

void ThreadScheduler::SetThreadStatusAndUpdateRunqueue(Thread *aThread, mtThreadStatus aStatus)
{
    uint8_t priority = aThread->GetPriority();

    if (aStatus >= THREAD_STATUS_RUNNING)
    {
        if (aThread->GetStatus() < THREAD_STATUS_RUNNING)
        {
            mtListNode *threadRunqueueEntry = aThread->GetRunqueueEntry();

            mSchedulerRunqueue[priority].RightPush(static_cast<Clist *>(threadRunqueueEntry));

            SetRunqueueBitCache(priority);
        }
    }
    else
    {
        if (aThread->GetStatus() >= THREAD_STATUS_RUNNING)
        {
            mSchedulerRunqueue[priority].LeftPop();

            if (mSchedulerRunqueue[priority].mNext == NULL)
            {
                ResetRunqueueBitCache(priority);
            }
        }
    }

    aThread->SetStatus(aStatus);
}

void ThreadScheduler::ContextSwitch(uint8_t aPriorityToSwitch)
{
    Thread *currentThread = GetCurrentActiveThread();

    uint8_t currentPriority = currentThread->GetPriority();

    int isInRunqueue = (currentThread->GetStatus() >= THREAD_STATUS_RUNNING);

    /* Note: the lowest priority number is the highest priority thread */

    if (!isInRunqueue || (currentPriority > aPriorityToSwitch))
    {
        if (mtCpuIsInIsr())
        {
            EnableContextSwitchRequestFromIsr();
        }
        else
        {
            YieldHigherPriorityThread();
        }
    }
}

void ThreadScheduler::YieldHigherPriorityThread(void)
{
    mtCpuTriggerPendSVInterrupt();
}

uint8_t ThreadScheduler::GetLSBIndexFromRunqueue(void)
{
    uint8_t index = 0;

    uint32_t queue = GetRunqueueBitCache();

    /* [IMPORTANT]: this functions assume there will be at least 1 thread on the queue, (idle) thread */

    while ((queue & 0x01) == 0)
    {
        queue >>= 1;
        index += 1;
    }

    return index;
}

Thread *ThreadScheduler::GetNextThreadFromRunqueue(void)
{
    uint8_t threadIdx = GetLSBIndexFromRunqueue();

    mtListNode *threadPtrInQueue = static_cast<mtListNode *>((mSchedulerRunqueue[threadIdx].mNext)->mNext);

    mtThread *thread = mtCONTAINER_OF(threadPtrInQueue, mtThread, mRunqueueEntry);

    return static_cast<Thread *>(thread);
}

void ThreadScheduler::SleepingCurrentThread(void)
{
    if (mtCpuIsInIsr())
    {
        return;
    }

    unsigned state = mtCpuIrqDisable();

    SetThreadStatusAndUpdateRunqueue(GetCurrentActiveThread(), THREAD_STATUS_SLEEPING);

    mtCpuIrqRestore(state);

    YieldHigherPriorityThread();
}

int ThreadScheduler::WakeupThread(mtKernelPid aPid)
{
    unsigned state = mtCpuIrqDisable();

    Thread *threadToWakeup = GetThreadFromScheduler(aPid);

    if (!threadToWakeup)
    {
        //TODO: Warning thread does not exist!
    }
    else if (threadToWakeup->GetStatus() == THREAD_STATUS_SLEEPING)
    {
        SetThreadStatusAndUpdateRunqueue(threadToWakeup, THREAD_STATUS_RUNNING);

        mtCpuIrqRestore(state);

        ContextSwitch(threadToWakeup->GetPriority());

        return 1;
    }
    else
    {
        // TODO: Warning thread is not sleeping!
    }

    mtCpuIrqRestore(state);

    return (int)THREAD_STATUS_NOT_FOUND;
}

void ThreadScheduler::Yield(void)
{
    unsigned state = mtCpuIrqDisable();

    Thread *currentThread = GetCurrentActiveThread();

    if (currentThread->GetStatus() >= THREAD_STATUS_RUNNING)
    {
        mSchedulerRunqueue[currentThread->GetPriority()].LeftPopRightPush();
    }

    mtCpuIrqRestore(state);

    YieldHigherPriorityThread();
}

const char *ThreadScheduler::ThreadStatusToString(mtThreadStatus aStatus)
{
    const char *retval;

    switch (aStatus)
    {
    case THREAD_STATUS_RUNNING:
        retval = "running";
        break;

    case THREAD_STATUS_PENDING:
        retval = "pending";
        break;

    case THREAD_STATUS_STOPPED:
        retval = "stopped";
        break;

    case THREAD_STATUS_SLEEPING:
        retval = "sleeping";
        break;

    case THREAD_STATUS_MUTEX_BLOCKED:
        retval = "bl mutex";
        break;

    case THREAD_STATUS_RECEIVE_BLOCKED:
        retval = "bl rx";
        break;

    case THREAD_STATUS_SEND_BLOCKED:
        retval = "bl send";
        break;

    case THREAD_STATUS_REPLY_BLOCKED:
        retval = "bl reply";
        break;

    default:
        retval = "unknown";
        break;
    }

    return retval;
}

extern "C" void mtCpuEndOfIsr(mtInstance *aInstance)
{
    Instance &instance = *static_cast<Instance *>(aInstance);

    if (instance.Get<ThreadScheduler>().IsContextSwitchRequestedFromIsr())
    {
        ThreadScheduler::YieldHigherPriorityThread();
    }
}

template <> inline Instance &Thread::Get(void) const
{
    return GetInstance();
}

template <typename Type> inline Type &Thread::Get(void) const
{
    return GetInstance().Get<Type>();
}

} // namespace mt
