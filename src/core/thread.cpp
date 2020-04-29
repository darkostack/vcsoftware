#include "core/locator-getters.hpp"
#include "core/thread.hpp"

namespace mt {

Thread *Thread::Init(Instance &aInstance,
                     char *aStack,
                     int aStackSize,
                     char aPriority,
                     int aFlags,
                     mtThreadHandlerFunc aHandlerFunc,
                     void *aArg,
                     const char *aName)
{
    if (aPriority >= MTOS_CONFIG_THREAD_SCHED_PRIO_LEVELS) return NULL;

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
        uintptr_t *stackp   = reinterpret_cast<uintptr_t *>(aStack);

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

    // TODO: unsigned state = mtCpuIrqDisable();

    /* initialize instances for this thread */
    tcb->InstanceLocatorInit::Init(aInstance);

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
        // TODO: mtCpuIrqRestore(state);
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

    tcb->InitMsgWaiters();

    tcb->Get<ThreadScheduler>().IncrementNumOfThreadsInScheduler();

    if (aFlags & THREAD_FLAGS_CREATE_SLEEPING)
    {
        tcb->Get<ThreadScheduler>().SetThreadStatusAndUpdateRunqueue(tcb, THREAD_STATUS_SLEEPING);
    }
    else
    {
        tcb->Get<ThreadScheduler>().SetThreadStatusAndUpdateRunqueue(tcb, THREAD_STATUS_PENDING);

        if (!(aFlags & THREAD_FLAGS_CREATE_WOUT_YIELD_OTHER_THREAD))
        {
            // TODO: mtCpuIrqRestore(state);

            tcb->Get<ThreadScheduler>().ContextSwitch(aPriority);

            return tcb;
        }
    }

    // TODO: mtCpuIrqRestore(state);

    return tcb;
}

void Thread::StackInit(mtThreadHandlerFunc aFunction, void *aArg, void *aStackStart, int aStackSize)
{
    (void) aFunction;
    (void) aArg;
    (void) aStackStart;
    (void) aStackSize;
}

void ThreadScheduler::Run(void)
{
    DisableContextSwitchRequest();

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
    (void) aPriorityToSwitch;
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

#define container_of(ptr, type, member) ({                    \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);  \
        (type *)( (char *)__mptr - offsetof(type,member) );})

Thread *ThreadScheduler::GetNextThreadFromRunqueue(void)
{
    uint8_t threadIdx = GetLSBIndexFromRunqueue();

    mtListNode *threadPtrInQueue = static_cast<mtListNode *>((mSchedulerRunqueue[threadIdx].mNext)->mNext);

    mtThread *thread = container_of(threadPtrInQueue, mtThread, mRunqueueEntry);

    return static_cast<Thread *>(thread);
}

} // namespace mt
