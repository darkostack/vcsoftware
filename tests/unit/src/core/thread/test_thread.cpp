#include "gtest/gtest.h"

#include "core/instance.hpp"
#include "core/code_utils.hpp"
#include "core/mutex.hpp"

#include "test-helper.h"

using namespace mt;

class TestThread : public testing::Test
{
protected:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

TEST_F(TestThread, singleThread)
{
    mtDEFINE_ALIGNED_VAR(buffer, sizeof(Instance), uint64_t);

    uint32_t size = mtARRAY_LENGTH(buffer);

    Instance instance = Instance::Init((void *)buffer, (size_t *)&size);

    EXPECT_TRUE(instance.IsInitialized());

    EXPECT_EQ(instance.Get<ThreadScheduler>().GetNumOfThreadsInScheduler(), 0);
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActiveThread(), nullptr);
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActivePid(), KERNEL_PID_UNDEF);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] create single thread and run the thread scheduler, that
     * thread is expected to be in running state and become current active
     * thread
     * -------------------------------------------------------------------------
     **/

    char stack[128];

    Thread *thread = Thread::Init(instance, stack, sizeof(stack), 15,
                                  THREAD_FLAGS_CREATE_WOUT_YIELD | \
                                  THREAD_FLAGS_CREATE_STACKMARKER,
                                  NULL, NULL, "idle");

    EXPECT_NE(thread, nullptr);

    EXPECT_EQ(thread->GetPid(), 1);
    EXPECT_EQ(thread->GetPriority(), 15);
    EXPECT_EQ(thread->GetName(), "idle");
    EXPECT_EQ(thread->GetStatus(), THREAD_STATUS_PENDING);

    EXPECT_EQ(instance.Get<ThreadScheduler>().GetNumOfThreadsInScheduler(), 1);
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetThreadFromScheduler(thread->GetPid()), thread);
    EXPECT_FALSE(instance.Get<ThreadScheduler>().IsContextSwitchRequestedFromIsr());
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActiveThread(), nullptr);
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActivePid(), KERNEL_PID_UNDEF);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(thread->GetStatus(), THREAD_STATUS_RUNNING);

    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActiveThread(), thread);
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActivePid(), thread->GetPid());
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetNumOfThreadsInScheduler(), 1);
}

TEST_F(TestThread, multipleThread)
{
    mtDEFINE_ALIGNED_VAR(buffer, sizeof(Instance), uint64_t);

    uint32_t size = mtARRAY_LENGTH(buffer);

    Instance instance = Instance::Init((void *)buffer, (size_t *)&size);

    EXPECT_TRUE(instance.IsInitialized());

    EXPECT_EQ(instance.Get<ThreadScheduler>().GetNumOfThreadsInScheduler(), 0);
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActiveThread(), nullptr);
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActivePid(), KERNEL_PID_UNDEF);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] create multiple thread ("idle" and "main" thread) and make
     * sure the thread with higher priority will be in running state and the
     * thread with lower priority ("idle" thread) is in pending state
     * -------------------------------------------------------------------------
     **/

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

    EXPECT_EQ(instance.Get<ThreadScheduler>().GetNumOfThreadsInScheduler(), 2);
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetThreadFromScheduler(idleThread->GetPid()), idleThread);
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetThreadFromScheduler(mainThread->GetPid()), mainThread);
    EXPECT_FALSE(instance.Get<ThreadScheduler>().IsContextSwitchRequestedFromIsr());
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActiveThread(), nullptr);
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActivePid(), KERNEL_PID_UNDEF);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);

    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActiveThread(), mainThread);
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActivePid(), mainThread->GetPid());
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetNumOfThreadsInScheduler(), 2);

    /* Note: at this point "main" thread is in running state and "idle" thread
     * is in pending state */

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] set the higher priority thread ("main" thread) to blocked
     * state and lower priority thread ("idle" thread) should be in in running
     * state
     * -------------------------------------------------------------------------
     **/

    instance.Get<ThreadScheduler>().SetThreadStatusAndUpdateRunqueue(mainThread, THREAD_STATUS_RECEIVE_BLOCKED);

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);

    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActiveThread(), mainThread);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_RUNNING);

    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActiveThread(), idleThread);
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActivePid(), idleThread->GetPid());
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetNumOfThreadsInScheduler(), 2);

    /* Note: at this point "main" thread is in blocked state and "idle" thread
     * is in blocked state */

    /**
     * -----------------------------------------------------------------------------
     * [TEST CASE] create new thread with higher priority than main and idle thread
     * and yield immediately
     * -----------------------------------------------------------------------------
     **/

    char task1Stack[128];

    Thread *task1Thread = Thread::Init(instance, task1Stack, sizeof(task1Stack), 5,
                                       THREAD_FLAGS_CREATE_STACKMARKER,
                                       NULL, NULL, "task1");

    EXPECT_NE(task1Thread, nullptr);

    EXPECT_EQ(task1Thread->GetPid(), 3);
    EXPECT_EQ(task1Thread->GetPriority(), 5);
    EXPECT_EQ(task1Thread->GetName(), "task1");
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_PENDING);

    /* Note: at this point cpu should immediately yield the "task1" by
     * triggering the PendSV interrupt and context switch from Isr is not requested */

    EXPECT_TRUE(testHelperIsPendSVInterruptTriggered());
    EXPECT_FALSE(instance.Get<ThreadScheduler>().IsContextSwitchRequestedFromIsr());

    instance.Get<ThreadScheduler>().Run();

    /* Note: after run the scheduler current active thread is expected to be
     * "task1" */

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);

    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActiveThread(), task1Thread);
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActivePid(), task1Thread->GetPid());
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetNumOfThreadsInScheduler(), 3);

    instance.Get<ThreadScheduler>().Yield(); /* this function will run higher priority thread */

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);

    /* task1Thread already the highest priority thread currently running, so
     * nothing change */

    EXPECT_EQ(ThreadScheduler::ThreadStatusToString(mainThread->GetStatus()), "bl rx");
    EXPECT_EQ(ThreadScheduler::ThreadStatusToString(idleThread->GetStatus()), "pending");
    EXPECT_EQ(ThreadScheduler::ThreadStatusToString(task1Thread->GetStatus()), "running");

    EXPECT_EQ(ThreadScheduler::ThreadStatusToString(THREAD_STATUS_STOPPED), "stopped");
    EXPECT_EQ(ThreadScheduler::ThreadStatusToString(THREAD_STATUS_SLEEPING), "sleeping");
    EXPECT_EQ(ThreadScheduler::ThreadStatusToString(THREAD_STATUS_MUTEX_BLOCKED), "bl mutex");
    EXPECT_EQ(ThreadScheduler::ThreadStatusToString(THREAD_STATUS_SEND_BLOCKED), "bl send");
    EXPECT_EQ(ThreadScheduler::ThreadStatusToString(THREAD_STATUS_REPLY_BLOCKED), "bl reply");
    EXPECT_EQ(ThreadScheduler::ThreadStatusToString(static_cast<mtThreadStatus>(100)), "unknown");

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] mutexes
     * -------------------------------------------------------------------------
     **/

    Mutex mutex = Mutex(instance);

    mutex.Lock();

    /* Note: mutex was unlocked, set to locked for the first time,
     * current active thread (task1Thread) will not change it's status until
     * second time `mutex.Lock()` is call */

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);

    mutex.Lock();

    /* Note: mutex was locked, calling `mutex.Lock()` for the second time in current active
     * thread (task1Thread), now it will set current active thread status to mutex_blocked and
     * yield to higher priority thread or remain thread */

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);

    mutex.Unlock();

    /* Note: this will unlock `task1Thread` and set to pending status and yield
     * to highest priority task */

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_PENDING);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);

    /* Note: set mainThread status to pending */

    instance.Get<ThreadScheduler>().SetThreadStatusAndUpdateRunqueue(mainThread, THREAD_STATUS_PENDING);

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);

    instance.Get<ThreadScheduler>().Run();

    mutex.Lock(); // call this in task1Thread : HEAD

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);

    mutex.Lock(); // call this in mainThread

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);

    /* Note: at this point both mainThread and task1Thread is in MUTEX LOCKED
     * status. */

    mutex.Unlock();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_PENDING);

    /* Note: task1Thread was in HEAD of the mutex queue, it will unlock this
     * task first. */

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);

    /* Note: task1Thread have higher priority than idleThread, so run
     * task1Thread. */

    mutex.Unlock();

    /* This will change mainThread status to pending */

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);

    /* Note: task1Thread has higher priority than mainThread, so keep running
     * task1Thread. */

    mutex.Unlock();

    /* No thread was waiting this mutex, set to NULL */

    EXPECT_EQ(mutex.mQueue.mNext, nullptr);

    /* Note: set mainThread status back to receive blocked */

    instance.Get<ThreadScheduler>().SetThreadStatusAndUpdateRunqueue(mainThread, THREAD_STATUS_RECEIVE_BLOCKED);

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] try to set current active thread to sleep and wakeup
     * -------------------------------------------------------------------------
     **/

    instance.Get<ThreadScheduler>().SleepingCurrentThread();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_SLEEPING);

    /* Note: at this point both mainThread and task1Thread are in blocking
     * status, so the next expected thread to run is idleThread because
     * idleThread was in pending status. */

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_SLEEPING);

    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActiveThread(), idleThread);
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActivePid(), idleThread->GetPid());
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetNumOfThreadsInScheduler(), 3);

    EXPECT_EQ(instance.Get<ThreadScheduler>().WakeupThread(mainThread->GetPid()), THREAD_STATUS_NOT_FOUND);

    /* Note: because mainThread is not is SLEEPING status, we expect nothing to
     * happen. */

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_SLEEPING);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_SLEEPING);

    /* Note: now we wakeup task1Thread which was in sleeping status */

    EXPECT_EQ(instance.Get<ThreadScheduler>().WakeupThread(task1Thread->GetPid()), true);

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);

    /* Note: note that both idleThread and task1Thread is in running status,
     * that's okay, after calling ThreadScheduler::Run() it will fixed threads
     * status and run highest priority thread. In this case we expect idleThread
     * should be in PENDING status and task1Thread in RUNNING status */

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] try to set sleep in Isr
     * -------------------------------------------------------------------------
     **/

    testHelperSetCpuInIsr(); // artificially set CPU in Isr

    instance.Get<ThreadScheduler>().SleepingCurrentThread();

    /* Note: we can't sleep in Isr functions so nothing is expected to happen */

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);

    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActiveThread(), task1Thread);
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActivePid(), task1Thread->GetPid());
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetNumOfThreadsInScheduler(), 3);

    testHelperResetCpuInIsr();

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] try to context swithing to lower priority thread than current
     * running thread
     * -------------------------------------------------------------------------
     **/

    testHelperResetPendSVTrigger(); /* reset the PendSV state */

    EXPECT_FALSE(testHelperIsPendSVInterruptTriggered());

    EXPECT_EQ(idleThread->GetPriority(), 15);
    EXPECT_EQ(mainThread->GetPriority(), 7);
    EXPECT_EQ(task1Thread->GetPriority(), 5); /* highest priority thread */

    instance.Get<ThreadScheduler>().ContextSwitch(mainThread->GetPriority());

    /* Note: because "mainThread" priority is lower than current running thread
     * and current running thread is still in running status, nothing should
     * happened */

    EXPECT_FALSE(testHelperIsPendSVInterruptTriggered());

    EXPECT_FALSE(instance.Get<ThreadScheduler>().IsContextSwitchRequestedFromIsr());

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);

    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActiveThread(), task1Thread);
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActivePid(), task1Thread->GetPid());
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetNumOfThreadsInScheduler(), 3);

    /**
     * ------------------------------------------------------------------------------
     * [TEST CASE] request context swicth inside Isr (Interrupt Service Routine) and
     * current running thread is not in running status, it expected to see context
     * switch is requested instead of yielding immediately to the next thread
     * ------------------------------------------------------------------------------
     **/

    /* Note: set "task1" at blocked state first and is expected to be the next thread to
     * run is "idle" thread */

    instance.Get<ThreadScheduler>().SetThreadStatusAndUpdateRunqueue(task1Thread, THREAD_STATUS_RECEIVE_BLOCKED);

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);

    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActiveThread(), idleThread);
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActivePid(), idleThread->GetPid());
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetNumOfThreadsInScheduler(), 3);

    /* Note: at this point idle thread is run as expected, because other
     * higher priority threads is in blocked state */

    testHelperSetCpuInIsr(); /* artificially set cpu in Isr */

    EXPECT_TRUE(mtCpuIsInIsr());

    instance.Get<ThreadScheduler>().ContextSwitch(task1Thread->GetPriority());

    EXPECT_FALSE(testHelperIsPendSVInterruptTriggered());

    EXPECT_TRUE(instance.Get<ThreadScheduler>().IsContextSwitchRequestedFromIsr());

    /* Note: because it is in Isr at this point context switch is requested
     * instead of immediatelly yield to "task1" */

    /* Note: in real cpu implementation, before exiting the Isr function it will
     * check this flag and trigger the PendSV interrupt if context switch is
     * requested, therefore after exiting Isr function PendSV interrupt will be
     * triggered and run thread scheduler */

    mtCpuEndOfIsr((mtInstance *)&instance);

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);

    testHelperResetCpuInIsr(); /* artificially out from Isr */

    EXPECT_FALSE(mtCpuIsInIsr());

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);

    /* Note: at this point the current active thread is still "idle" because "task1"
     * is still in receive blocked state even though it was try to context
     * switch to "task1", now try set "task1" to pending state and
     * context switch to "task1" priority */

    instance.Get<ThreadScheduler>().SetThreadStatusAndUpdateRunqueue(task1Thread, THREAD_STATUS_PENDING);

    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_PENDING);

    instance.Get<ThreadScheduler>().ContextSwitch(task1Thread->GetPriority());

    EXPECT_TRUE(testHelperIsPendSVInterruptTriggered());

    EXPECT_FALSE(instance.Get<ThreadScheduler>().IsContextSwitchRequestedFromIsr());

    instance.Get<ThreadScheduler>().Run();

    testHelperResetPendSVTrigger(); /* artificially reset PendSV state */

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);

    /* Note: at this point it succesfully switched to "task1" */

    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActiveThread(), task1Thread);
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActivePid(), task1Thread->GetPid());
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetNumOfThreadsInScheduler(), 3);

    /**
     * ------------------------------------------------------------------------------
     * [TEST CASE] create a thread with highest priority but with THREAD_FLAGS
     * sleeping and not using THREAD_FLAGS create stack marker.
     * ------------------------------------------------------------------------------
     **/

    char task2Stack[128];

    /* Note: intentionally create thread with misaligment stack boundary on
     * 16/32 bit boundary (&task2Stack[1]) will do the job */

    Thread *task2Thread = Thread::Init(instance, &task2Stack[1], sizeof(task2Stack), 1,
                                       THREAD_FLAGS_CREATE_SLEEPING,
                                       NULL, NULL, "task2");

    EXPECT_NE(task2Thread, nullptr);

    EXPECT_EQ(task2Thread->GetPid(), 4);
    EXPECT_EQ(task2Thread->GetPriority(), 1);
    EXPECT_EQ(task2Thread->GetName(), "task2");
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_SLEEPING);

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_SLEEPING);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_SLEEPING);

    instance.Get<ThreadScheduler>().WakeupThread(task2Thread->GetPid());

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_RUNNING);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task2Thread->GetStatus(), THREAD_STATUS_RUNNING);
}

TEST_F(TestThread, multipleInstanceMultiThreads)
{
    mtDEFINE_ALIGNED_VAR(buffer1, sizeof(Instance), uint64_t);
    mtDEFINE_ALIGNED_VAR(buffer2, sizeof(Instance), uint64_t);

    uint32_t size1 = mtARRAY_LENGTH(buffer1);
    uint32_t size2 = mtARRAY_LENGTH(buffer2);

    Instance instance1 = Instance::Init((void *)buffer1, (size_t *)&size1);
    Instance instance2 = Instance::Init((void *)buffer2, (size_t *)&size2);

    EXPECT_TRUE(instance1.IsInitialized());
    EXPECT_TRUE(instance2.IsInitialized());

    EXPECT_EQ(instance1.Get<ThreadScheduler>().GetNumOfThreadsInScheduler(), 0);
    EXPECT_EQ(instance1.Get<ThreadScheduler>().GetCurrentActiveThread(), nullptr);
    EXPECT_EQ(instance1.Get<ThreadScheduler>().GetCurrentActivePid(), KERNEL_PID_UNDEF);

    EXPECT_EQ(instance2.Get<ThreadScheduler>().GetNumOfThreadsInScheduler(), 0);
    EXPECT_EQ(instance2.Get<ThreadScheduler>().GetCurrentActiveThread(), nullptr);
    EXPECT_EQ(instance2.Get<ThreadScheduler>().GetCurrentActivePid(), KERNEL_PID_UNDEF);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] create multiple instances and multiple threads in those
     * instances
     * -------------------------------------------------------------------------
     **/

    /* Note: create "thread1" and "thread2" in instance1 */

    char instance1Stack1[128];
    char instance1Stack2[128];

    Thread *instance1Thread1 = Thread::Init(instance1, instance1Stack1, sizeof(instance1Stack1), 15,
                                            THREAD_FLAGS_CREATE_WOUT_YIELD | \
                                            THREAD_FLAGS_CREATE_STACKMARKER,
                                            NULL, NULL, "thread1");

    Thread *instance1Thread2 = Thread::Init(instance1, instance1Stack2, sizeof(instance1Stack2), 15,
                                            THREAD_FLAGS_CREATE_WOUT_YIELD | \
                                            THREAD_FLAGS_CREATE_STACKMARKER,
                                            NULL, NULL, "thread2");

    EXPECT_NE(instance1Thread1, nullptr);
    EXPECT_NE(instance1Thread2, nullptr);

    EXPECT_EQ(instance1Thread1->GetPid(), 1);
    EXPECT_EQ(instance1Thread1->GetPriority(), 15);
    EXPECT_EQ(instance1Thread1->GetName(), "thread1");
    EXPECT_EQ(instance1Thread1->GetStatus(), THREAD_STATUS_PENDING);

    EXPECT_EQ(instance1Thread2->GetPid(), 2);
    EXPECT_EQ(instance1Thread2->GetPriority(), 15);
    EXPECT_EQ(instance1Thread2->GetName(), "thread2");
    EXPECT_EQ(instance1Thread2->GetStatus(), THREAD_STATUS_PENDING);

    EXPECT_EQ(instance1.Get<ThreadScheduler>().GetNumOfThreadsInScheduler(), 2);
    EXPECT_EQ(instance1.Get<ThreadScheduler>().GetThreadFromScheduler(1), instance1Thread1);
    EXPECT_EQ(instance1.Get<ThreadScheduler>().GetThreadFromScheduler(2), instance1Thread2);
    EXPECT_FALSE(instance1.Get<ThreadScheduler>().IsContextSwitchRequestedFromIsr());
    EXPECT_EQ(instance1.Get<ThreadScheduler>().GetCurrentActiveThread(), nullptr);
    EXPECT_EQ(instance1.Get<ThreadScheduler>().GetCurrentActivePid(), KERNEL_PID_UNDEF);

    instance1.Get<ThreadScheduler>().Run();

    EXPECT_EQ(instance1Thread1->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(instance1Thread2->GetStatus(), THREAD_STATUS_PENDING);

    EXPECT_EQ(instance1.Get<ThreadScheduler>().GetCurrentActiveThread(), instance1Thread1);
    EXPECT_EQ(instance1.Get<ThreadScheduler>().GetCurrentActivePid(), instance1Thread1->GetPid());
    EXPECT_EQ(instance1.Get<ThreadScheduler>().GetNumOfThreadsInScheduler(), 2);

    /* Note: create "thread1" and "thread2" in instance2 */

    char instance2Stack1[128];
    char instance2Stack2[128];

    Thread *instance2Thread1 = Thread::Init(instance2, instance2Stack1, sizeof(instance2Stack1), 15,
                                            THREAD_FLAGS_CREATE_WOUT_YIELD | \
                                            THREAD_FLAGS_CREATE_STACKMARKER,
                                            NULL, NULL, "thread1");

    Thread *instance2Thread2 = Thread::Init(instance2, instance2Stack2, sizeof(instance2Stack2), 15,
                                            THREAD_FLAGS_CREATE_WOUT_YIELD | \
                                            THREAD_FLAGS_CREATE_STACKMARKER,
                                            NULL, NULL, "thread2");

    EXPECT_NE(instance2Thread1, nullptr);
    EXPECT_NE(instance2Thread2, nullptr);

    EXPECT_EQ(instance2Thread1->GetPid(), 1);
    EXPECT_EQ(instance2Thread1->GetPriority(), 15);
    EXPECT_EQ(instance2Thread1->GetName(), "thread1");
    EXPECT_EQ(instance2Thread1->GetStatus(), THREAD_STATUS_PENDING);

    EXPECT_EQ(instance2Thread2->GetPid(), 2);
    EXPECT_EQ(instance2Thread2->GetPriority(), 15);
    EXPECT_EQ(instance2Thread2->GetName(), "thread2");
    EXPECT_EQ(instance2Thread2->GetStatus(), THREAD_STATUS_PENDING);

    EXPECT_EQ(instance2.Get<ThreadScheduler>().GetNumOfThreadsInScheduler(), 2);
    EXPECT_EQ(instance2.Get<ThreadScheduler>().GetThreadFromScheduler(1), instance2Thread1);
    EXPECT_EQ(instance2.Get<ThreadScheduler>().GetThreadFromScheduler(2), instance2Thread2);
    EXPECT_FALSE(instance2.Get<ThreadScheduler>().IsContextSwitchRequestedFromIsr());
    EXPECT_EQ(instance2.Get<ThreadScheduler>().GetCurrentActiveThread(), nullptr);
    EXPECT_EQ(instance2.Get<ThreadScheduler>().GetCurrentActivePid(), KERNEL_PID_UNDEF);

    instance2.Get<ThreadScheduler>().Run();

    EXPECT_EQ(instance2Thread1->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(instance2Thread2->GetStatus(), THREAD_STATUS_PENDING);

    EXPECT_EQ(instance2.Get<ThreadScheduler>().GetCurrentActiveThread(), instance2Thread1);
    EXPECT_EQ(instance2.Get<ThreadScheduler>().GetCurrentActivePid(), instance2Thread1->GetPid());
    EXPECT_EQ(instance2.Get<ThreadScheduler>().GetNumOfThreadsInScheduler(), 2);

    /**
     * ------------------------------------------------------------------------------
     * [TEST CASE] set "thread2" in instance2 to pending state, since "thread1" and
     * "thread2" have the same priority all of those thread will not run
     * unless one of those thread in blocked state and expected to run the thread
     * with the pending state or status 
     * ------------------------------------------------------------------------------
     **/

    instance2.Get<ThreadScheduler>().SetThreadStatusAndUpdateRunqueue(instance2Thread1, THREAD_STATUS_PENDING);

    EXPECT_EQ(instance2Thread1->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(instance2Thread2->GetStatus(), THREAD_STATUS_PENDING);

    instance2.Get<ThreadScheduler>().Run();

    EXPECT_EQ(instance2Thread1->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(instance2Thread2->GetStatus(), THREAD_STATUS_PENDING);

    instance2.Get<ThreadScheduler>().Run();

    EXPECT_EQ(instance2Thread1->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(instance2Thread2->GetStatus(), THREAD_STATUS_PENDING);

    instance2.Get<ThreadScheduler>().Run();

    EXPECT_EQ(instance2Thread1->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(instance2Thread2->GetStatus(), THREAD_STATUS_PENDING);

    instance2.Get<ThreadScheduler>().SetThreadStatusAndUpdateRunqueue(instance2Thread1, THREAD_STATUS_RECEIVE_BLOCKED);

    EXPECT_EQ(instance2Thread1->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(instance2Thread2->GetStatus(), THREAD_STATUS_PENDING);

    instance2.Get<ThreadScheduler>().Run();

    EXPECT_EQ(instance2Thread1->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(instance2Thread2->GetStatus(), THREAD_STATUS_RUNNING);

    EXPECT_EQ(instance2.Get<ThreadScheduler>().GetCurrentActiveThread(), instance2Thread2);
    EXPECT_EQ(instance2.Get<ThreadScheduler>().GetCurrentActivePid(), instance2Thread2->GetPid());
    EXPECT_EQ(instance2.Get<ThreadScheduler>().GetNumOfThreadsInScheduler(), 2);
}
