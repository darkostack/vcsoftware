#include <assert.h>

#include "core/locator-getters.hpp"
#include "core/msg.hpp"

namespace mt {

int Msg::Send(mtKernelPid aTargetPid, int aBlocking, unsigned aState)
{
    if (!Thread::IsPidValid(aTargetPid))
    {
        //TODO: warning pid it not valid
    }

    Thread *targetThread = Get<ThreadScheduler>().GetThreadFromScheduler(aTargetPid);

    mSenderPid = Get<ThreadScheduler>().GetCurrentActivePid();

    if (targetThread == NULL)
    {
        mtCpuIrqRestore(aState);
        return -1;
    }

    Thread *currentThread = Get<ThreadScheduler>().GetCurrentActiveThread();

    if (targetThread->GetStatus() != THREAD_STATUS_RECEIVE_BLOCKED)
    {
        if (targetThread->QueuedMsg(this))
        {
            mtCpuIrqRestore(aState);

            if (currentThread->GetStatus() == THREAD_STATUS_REPLY_BLOCKED)
            {
                ThreadScheduler::YieldHigherPriorityThread();
            }

            return 1;
        }

        if (!aBlocking)
        {
            mtCpuIrqRestore(aState);
            return 0;
        }

        currentThread->mWaitData = static_cast<void *>(this);

        mtThreadStatus newStatus;

        if (currentThread->GetStatus() == THREAD_STATUS_REPLY_BLOCKED)
        {
            newStatus = THREAD_STATUS_REPLY_BLOCKED;
        }
        else
        {
            newStatus = THREAD_STATUS_SEND_BLOCKED;
        }

        Get<ThreadScheduler>().SetThreadStatusAndUpdateRunqueue(currentThread, newStatus);

        currentThread->AddToList(static_cast<List *>(&targetThread->mMsgWaiters));

        mtCpuIrqRestore(aState);

        ThreadScheduler::YieldHigherPriorityThread();
    }
    else
    {
        Msg *targetMsg = static_cast<Msg *>(targetThread->mWaitData);

        *targetMsg = *this;

        Get<ThreadScheduler>().SetThreadStatusAndUpdateRunqueue(targetThread, THREAD_STATUS_PENDING);

        mtCpuIrqRestore(aState);

        ThreadScheduler::YieldHigherPriorityThread();
    }

    return 1;
}

int Msg::Receive(int aBlocking)
{
    unsigned state = mtCpuIrqDisable();

    Thread *currentThread = Get<ThreadScheduler>().GetCurrentActiveThread();

    int queueIndex = -1;

    if (currentThread->mMsgArray != NULL)
    {
        queueIndex = (static_cast<Cib *>(&currentThread->mMsgQueue))->Get();
    }

    if (!aBlocking && (!currentThread->mMsgWaiters.mNext && queueIndex == -1))
    {
        mtCpuIrqRestore(state);
        return -1;
    }

    if (queueIndex >= 0)
    {
        *this = *static_cast<Msg *>(&currentThread->mMsgArray[queueIndex]);
    }
    else
    {
        currentThread->mWaitData = static_cast<void *>(this);
    }

    List *next = (static_cast<List *>(&currentThread->mMsgWaiters))->RemoveHead();

    if (next == NULL)
    {
        if (queueIndex < 0)
        {
            Get<ThreadScheduler>().SetThreadStatusAndUpdateRunqueue(currentThread, THREAD_STATUS_RECEIVE_BLOCKED);

            mtCpuIrqRestore(state);

            ThreadScheduler::YieldHigherPriorityThread();

            assert(Get<ThreadScheduler>().GetCurrentActiveThread()->GetStatus() != THREAD_STATUS_RECEIVE_BLOCKED);
        }
        else
        {
            mtCpuIrqRestore(state);
        }

        return 1;
    }
    else
    {
        Thread *senderThread = Thread::GetThreadPointerFromItsListMember(next);

        Msg *tmp = NULL;

        if (queueIndex >= 0)
        {
            /* We've already got a message from the queue. As there is a
             * waiter, take it's message into the just freed queue space. */
            tmp = static_cast<Msg *>(&currentThread->mMsgArray[(static_cast<Cib *>(&currentThread->mMsgQueue))->Put()]);
        }

        /* copy msg */
        Msg *senderMsg = static_cast<Msg *>(senderThread->mWaitData);

        if (tmp != NULL)
        {
            *tmp = *senderMsg;
            *this = *tmp;
        }
        else
        {
            *this = *senderMsg;
        }

        /* remove sender from queue */
        uint8_t senderPriority = KERNEL_THREAD_PRIORITY_IDLE;

        if (senderThread->GetStatus() != THREAD_STATUS_REPLY_BLOCKED)
        {
            senderThread->mWaitData = NULL;

            Get<ThreadScheduler>().SetThreadStatusAndUpdateRunqueue(senderThread, THREAD_STATUS_PENDING);

            senderPriority = senderThread->GetPriority();
        }

        mtCpuIrqRestore(state);

        if (senderPriority < KERNEL_THREAD_PRIORITY_IDLE)
        {
            Get<ThreadScheduler>().ContextSwitch(senderPriority);
        }

        return 1;
    }
}

int Msg::Send(mtKernelPid aTargetPid)
{
    if (mtCpuIsInIsr())
    {
        return SendInIsr(aTargetPid);
    }

    if (Get<ThreadScheduler>().GetCurrentActivePid() == aTargetPid)
    {
        return SendToSelfQueue();
    }

    return Send(aTargetPid, 1 /* blocking */, mtCpuIrqDisable());
}

int Msg::TrySend(mtKernelPid aTargetPid)
{
    if (mtCpuIsInIsr())
    {
        return SendInIsr(aTargetPid);
    }

    if (Get<ThreadScheduler>().GetCurrentActivePid() == aTargetPid)
    {
        return SendToSelfQueue();
    }

    return Send(aTargetPid, 0 /* non-blocking */, mtCpuIrqDisable());
}

int Msg::SendToSelfQueue(void)
{
    unsigned state = mtCpuIrqDisable();

    Thread *currentThread = Get<ThreadScheduler>().GetCurrentActiveThread();

    mSenderPid = currentThread->GetPid();

    int result = currentThread->QueuedMsg(this);

    mtCpuIrqRestore(state);

    return result;
}

int Msg::SendInIsr(mtKernelPid aTargetPid)
{
    if (!Thread::IsPidValid(aTargetPid))
    {
        // TODO: warning pit is not valid
    }

    Thread *targetThread = Get<ThreadScheduler>().GetThreadFromScheduler(aTargetPid);

    if (targetThread == NULL)
    {
        return -1;
    }

    mSenderPid = KERNEL_PID_ISR;

    if (targetThread->GetStatus() == THREAD_STATUS_RECEIVE_BLOCKED)
    {
        Msg *targetMsg = static_cast<Msg *>(targetThread->mWaitData);

        *targetMsg = *this;

        Get<ThreadScheduler>().SetThreadStatusAndUpdateRunqueue(targetThread, THREAD_STATUS_PENDING);

        Get<ThreadScheduler>().EnableContextSwitchRequestFromIsr();

        return 1;
    }
    else
    {
        return targetThread->QueuedMsg(this);
    }
}

int Msg::SendReceive(Msg *aReply, mtKernelPid aTargetPid)
{
    assert(Get<ThreadScheduler>().GetCurrentActivePid() != aTargetPid);

    unsigned state = mtCpuIrqDisable();

    Thread *currentThread = Get<ThreadScheduler>().GetCurrentActiveThread();

    Get<ThreadScheduler>().SetThreadStatusAndUpdateRunqueue(currentThread, THREAD_STATUS_REPLY_BLOCKED);

    currentThread->mWaitData = static_cast<void *>(aReply);

    /* we re-use (abuse) reply for sending, because wait_data might be
     * overwritten if the target is not in RECEIVE_BLOCKED */

    *aReply = *this;

    /* Send() blocks until reply received */
    return aReply->Send(aTargetPid, 1 /* blocking */, state);
}

int Msg::Reply(Msg *aReply)
{
    unsigned state = mtCpuIrqDisable();

    Thread *targetThread = Get<ThreadScheduler>().GetThreadFromScheduler(mSenderPid);

    assert(targetThread != NULL);

    if (targetThread->GetStatus() != THREAD_STATUS_REPLY_BLOCKED)
    {
        mtCpuIrqRestore(state);

        return -1;
    }

    Msg *targetMsg = static_cast<Msg *>(targetThread->mWaitData);

    *targetMsg = *aReply;

    Get<ThreadScheduler>().SetThreadStatusAndUpdateRunqueue(targetThread, THREAD_STATUS_PENDING);

    uint8_t targetPrio = targetThread->GetPriority();

    mtCpuIrqRestore(state);

    Get<ThreadScheduler>().ContextSwitch(targetPrio);

    return 1;
}

int Msg::ReplyInIsr(Msg *aReply)
{
    Thread *targetThread = Get<ThreadScheduler>().GetThreadFromScheduler(mSenderPid);

    if (targetThread->GetStatus() != THREAD_STATUS_REPLY_BLOCKED)
    {
        return -1;
    }

    Msg *targetMsg = static_cast<Msg *>(targetThread->mWaitData);

    *targetMsg = *aReply;

    Get<ThreadScheduler>().SetThreadStatusAndUpdateRunqueue(targetThread, THREAD_STATUS_PENDING);

    Get<ThreadScheduler>().EnableContextSwitchRequestFromIsr();

    return 1;
}

} // namespace mt
