#include "gtest/gtest.h"

#include "core/instance.hpp"
#include "core/code_utils.hpp"
#include "core/mutex.hpp"

#include "test-helper.h"

using namespace mt;

class TestMutex : public testing::Test
{
protected:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

TEST_F(TestMutex, singleInstanceMutex)
{
    mtDEFINE_ALIGNED_VAR(buffer, sizeof(Instance), uint64_t);

    uint32_t size = mtARRAY_LENGTH(buffer);

    Instance instance = Instance::Init((void *)buffer, (size_t *)&size);

    EXPECT_TRUE(instance.IsInitialized());

    EXPECT_EQ(instance.Get<ThreadScheduler>().GetNumOfThreadsInScheduler(), 0);
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActiveThread(), nullptr);
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActivePid(), KERNEL_PID_UNDEF);

    char idleStack[128];

    Thread *idleThread = Thread::Init(instance, idleStack, sizeof(idleStack), 15,
                                      THREAD_FLAGS_CREATE_WOUT_YIELD | \
                                      THREAD_FLAGS_CREATE_STACKMARKER,
                                      NULL, NULL, "idle");

    EXPECT_NE(idleThread, nullptr);

    EXPECT_EQ(idleThread->GetPid(), 1);
    EXPECT_EQ(idleThread->GetPriority(), 15);
    EXPECT_EQ(idleThread->GetName(), "idle");
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);

    char mainStack[128];

    Thread *mainThread = Thread::Init(instance, mainStack, sizeof(mainStack), 7,
                                      THREAD_FLAGS_CREATE_WOUT_YIELD | \
                                      THREAD_FLAGS_CREATE_STACKMARKER,
                                      NULL, NULL, "main");

    EXPECT_NE(mainThread, nullptr);

    EXPECT_EQ(mainThread->GetPid(), 2);
    EXPECT_EQ(mainThread->GetPriority(), 7);
    EXPECT_EQ(mainThread->GetName(), "main");
    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);

    char task1Stack[128];

    Thread *task1Thread = Thread::Init(instance, task1Stack, sizeof(task1Stack), 5,
                                       THREAD_FLAGS_CREATE_WOUT_YIELD | \
                                       THREAD_FLAGS_CREATE_STACKMARKER,
                                       NULL, NULL, "task1");

    EXPECT_NE(task1Thread, nullptr);

    EXPECT_EQ(task1Thread->GetPid(), 3);
    EXPECT_EQ(task1Thread->GetPriority(), 5);
    EXPECT_EQ(task1Thread->GetName(), "task1");
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_PENDING);

    char task2Stack[128];

    Thread *task2Thread = Thread::Init(instance, task2Stack, sizeof(task1Stack), 5,
                                       THREAD_FLAGS_CREATE_WOUT_YIELD | \
                                       THREAD_FLAGS_CREATE_STACKMARKER,
                                       NULL, NULL, "task2");

    EXPECT_NE(task2Thread, nullptr);

    EXPECT_EQ(task2Thread->GetPid(), 4);
    EXPECT_EQ(task2Thread->GetPriority(), 5);
    EXPECT_EQ(task2Thread->GetName(), "task2");
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_PENDING);

    EXPECT_EQ(instance.Get<ThreadScheduler>().GetNumOfThreadsInScheduler(), 4);
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetThreadFromScheduler(idleThread->GetPid()), idleThread);
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetThreadFromScheduler(mainThread->GetPid()), mainThread);
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetThreadFromScheduler(task1Thread->GetPid()), task1Thread);
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetThreadFromScheduler(task2Thread->GetPid()), task2Thread);
    EXPECT_FALSE(instance.Get<ThreadScheduler>().IsContextSwitchRequestedFromIsr());
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActiveThread(), nullptr);
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActivePid(), KERNEL_PID_UNDEF);

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_PENDING);

    /* Note: because task1Thread created first compare to task2Thread, therefore
     * task1Thread will running first although they have same priority. */

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_PENDING);

    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActiveThread(), task1Thread);
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActivePid(), task1Thread->GetPid());
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetNumOfThreadsInScheduler(), 4);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_PENDING);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] single mutex
     * -------------------------------------------------------------------------
     **/

    Mutex mutex = Mutex(instance);

    mutex.Lock();

    /* Note: mutex was unlocked, set to locked for the first time and still
     * running current thread */

    mutex.TryLock();
    mutex.TryLock();
    mutex.TryLock();
    mutex.TryLock();

    /* Note: mutex already in locked, so nothing happen when calling TryLock() at this point */

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_PENDING);

    mutex.Lock(); /* this will blocked task1Thread */

    /* Note: mutex was locked, set current thread status to mutex blocked */

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_PENDING);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_RUNNING);

    mutex.Lock(); /* this will blocked task2Thread */

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);

    mutex.Unlock(); /* this will unlocked task1Thread */

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);

    mutex.Unlock(); /* this will unlocked task2Thread */

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_PENDING);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_PENDING);

    /* Note: task1Thread and task2Thread have the same priority so it will keep
     * running task1Thread. */

    mutex.Unlock(); /* no body wait for this mutex, set to null */

    EXPECT_EQ(mutex.mQueue.mNext, nullptr);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] multiple mutexes
     * -------------------------------------------------------------------------
     **/

    Mutex mutex1 = Mutex(instance);
    Mutex mutex2 = Mutex(instance);
    Mutex mutex3 = Mutex(instance);

    mutex1.Lock();
    mutex2.Lock();
    mutex3.Lock();

    /* Note: all mutexes was unlocked, now set to locked */

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_PENDING);

    mutex1.Lock(); /* call this mutex in task1Thread */

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_PENDING);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_RUNNING);

    mutex2.Lock(); /* call this mutex in task2Thread */

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);

    mutex3.Lock(); /* call this mutex in mainThread */

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);

    mutex2.Unlock(); /* this will set task2Thread to pending status */

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_PENDING);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_RUNNING);

    mutex1.Unlock(); /* this will set task1Thread to pending status but keep running task2Thread */

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_RUNNING);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_RUNNING);

    mutex3.Unlock(); /* this will set mainThread to pending status */

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_RUNNING);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_RUNNING);

    mutex3.Lock(); /* this will set task2Thread to mutex blocked status */

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);

    mutex3.Unlock();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_PENDING);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_PENDING);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] unlock mutex in Isr
     * -------------------------------------------------------------------------
     **/

    mutex3.Lock(); /* this will set task1Thread to mutex blocked status */

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_PENDING);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_RUNNING);

    testHelperSetCpuInIsr(); // artificially set CPU in Isr

    mutex3.Unlock();

    testHelperResetCpuInIsr();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_RUNNING);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_RUNNING);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] unlock and sleeping current thread
     * -------------------------------------------------------------------------
     **/

    mutex3.Lock(); /* this will set task2Thread to mutex blocked status */

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);

    mutex3.UnlockAndSleepingCurrentThread(); /* this will unlock task2Thread and set task1Thread to sleeping */

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_SLEEPING);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_PENDING);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_SLEEPING);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_RUNNING);

    mutex3.UnlockAndSleepingCurrentThread();

    /* Note: No thread was blocked by mutex3, nothing to unlocked but keep set
     * the currentThread to sleeping */

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_SLEEPING);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_SLEEPING);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_SLEEPING);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_SLEEPING);

    mutex3.UnlockAndSleepingCurrentThread();

    /* Note: No thread was blocked by mutex3, nothing to unlocked but keep set
     * the currentThread to sleeping */

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_SLEEPING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_SLEEPING);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_SLEEPING);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_SLEEPING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_SLEEPING);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_SLEEPING);

    mutex3.Unlock();
    mutex3.Unlock();
    mutex3.Unlock();
    mutex3.Unlock();
    mutex3.Unlock();
    mutex3.Unlock();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_SLEEPING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_SLEEPING);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_SLEEPING);

    /* Note: mutex3 was unlocked, nothing happen */
}
