#include "core/locator-getters.hpp"
#include "core/mutex.hpp"
#include "core/thread.hpp"

namespace mt {

int Mutex::SetLock(int aBlocking)
{
    unsigned state = mtCpuIrqDisable();

    if (mQueue.mNext == NULL)
    {
        /* mutex was unlocked */
        mQueue.mNext = MUTEX_LOCKED;

        mtCpuIrqRestore(state);

        return 1;
    }
    else if (aBlocking)
    {
        Thread *currentThread = Get<ThreadScheduler>().GetCurrentActiveThread();

        Get<ThreadScheduler>().SetThreadStatusAndUpdateRunqueue(currentThread, THREAD_STATUS_MUTEX_BLOCKED);

        if (mQueue.mNext == MUTEX_LOCKED)
        {
            mQueue.mNext = currentThread->GetRunqueueEntry();
            mQueue.mNext->mNext = NULL;
        }
        else
        {
            currentThread->AddToList(static_cast<List *>(&mQueue));
        }

        mtCpuIrqRestore(state);

        Get<ThreadScheduler>().YieldHigherPriorityThread();

        return 1;
    }
    else
    {
        mtCpuIrqRestore(state);

        return 0;
    }
}

void Mutex::Unlock(void)
{
    unsigned state = mtCpuIrqDisable();

    if (mQueue.mNext == NULL)
    {
        /* mutex was unlocked */
        mtCpuIrqRestore(state);
        return;
    }

    if (mQueue.mNext == MUTEX_LOCKED)
    {
        mQueue.mNext = NULL;
        /* mutex was locked but no thread was waiting for it */
        mtCpuIrqRestore(state);
        return;
    }

    List *next = (static_cast<List *>(&mQueue))->RemoveHead();

    Thread *thread = Thread::GetThreadPointerFromItsListMember(next);

    Get<ThreadScheduler>().SetThreadStatusAndUpdateRunqueue(thread, THREAD_STATUS_PENDING);

    if (!mQueue.mNext)
    {
        mQueue.mNext = MUTEX_LOCKED;
    }

    uint8_t threadPriority = thread->GetPriority();

    mtCpuIrqRestore(state);

    Get<ThreadScheduler>().ContextSwitch(threadPriority);
}

void Mutex::UnlockAndSleepingCurrentThread(void)
{
    unsigned state = mtCpuIrqDisable();

    if (mQueue.mNext)
    {
        if (mQueue.mNext == MUTEX_LOCKED)
        {
            mQueue.mNext = NULL;
        }
        else
        {
            List *next = (static_cast<List *>(&mQueue))->RemoveHead();

            Thread *thread = Thread::GetThreadPointerFromItsListMember(next);

            Get<ThreadScheduler>().SetThreadStatusAndUpdateRunqueue(thread, THREAD_STATUS_PENDING);

            if (!mQueue.mNext)
            {
                mQueue.mNext = MUTEX_LOCKED;
            }
        }
    }

    mtCpuIrqRestore(state);

    Get<ThreadScheduler>().SleepingCurrentThread();
}

} // namespace mt
