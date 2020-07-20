#include "gtest/gtest.h"

#include "core/instance.hpp"
#include "core/code_utils.hpp"
#include "core/locator-getters.hpp"
#include "core/mutex.hpp"
#include "core/msg.hpp"

#include "test-helper.h"

using namespace mt;

class TestMsg : public testing::Test
{
protected:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

TEST_F(TestMsg, singleSendAndReceiveMsg)
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

    EXPECT_EQ(instance.Get<ThreadScheduler>().GetNumOfThreadsInScheduler(), 3);
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetThreadFromScheduler(idleThread->GetPid()), idleThread);
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetThreadFromScheduler(mainThread->GetPid()), mainThread);
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetThreadFromScheduler(task1Thread->GetPid()), task1Thread);
    EXPECT_FALSE(instance.Get<ThreadScheduler>().IsContextSwitchRequestedFromISR());
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActiveThread(), nullptr);
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActivePid(), KERNEL_PID_UNDEF);

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_PENDING);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);

    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActiveThread(), task1Thread);
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActivePid(), task1Thread->GetPid());
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetNumOfThreadsInScheduler(), 3);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] basic send and receive message
     * -------------------------------------------------------------------------
     **/

    Msg msg1 = Msg(instance);

    EXPECT_EQ(msg1.mSenderPid, KERNEL_PID_UNDEF);
    EXPECT_EQ(msg1.mType, 0);
    EXPECT_EQ(msg1.mContent.mPtr, nullptr);
    EXPECT_EQ(msg1.mContent.mValue, 0);

    /* call msg.Receive() in current thread (task1Thread), it expected to set
     * current thread status to receive blocking */

    msg1.Receive();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);

    Msg msg2 = Msg(instance);

    EXPECT_EQ(msg2.mSenderPid, KERNEL_PID_UNDEF);
    EXPECT_EQ(msg2.mType, 0);
    EXPECT_EQ(msg2.mContent.mPtr, nullptr);
    EXPECT_EQ(msg2.mContent.mValue, 0);

    uint32_t msgValue = 0xdeadbeef;

    msg2.mType = 0x20;
    msg2.mContent.mPtr = static_cast<void *>(&msgValue);

    EXPECT_EQ(msg2.Send(task1Thread->GetPid()), 1);

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_PENDING);

    /* Note: at this point, msg2 is immediately received by task1Thread because
     * it was in RECEIVE_BLOCKED status */

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);

    EXPECT_EQ(msg1.mSenderPid, mainThread->GetPid());
    EXPECT_EQ(msg1.mType, 0x20);
    EXPECT_NE(msg1.mContent.mPtr, nullptr);
    EXPECT_EQ(*static_cast<uint32_t *>(msg1.mContent.mPtr), 0xdeadbeef);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] send message to a thread that doesn't in receive blocked
     * -------------------------------------------------------------------------
     **/

    Msg msg3 = Msg(instance);

    EXPECT_EQ(msg3.mSenderPid, KERNEL_PID_UNDEF);
    EXPECT_EQ(msg3.mType, 0);
    EXPECT_EQ(msg3.mContent.mPtr, nullptr);
    EXPECT_EQ(msg3.mContent.mValue, 0);

    msg3.mType = 0x21;
    msg3.mContent.mValue = 0xdeadbeef;

    EXPECT_EQ(msg3.Send(mainThread->GetPid()), 1);

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_SEND_BLOCKED);

    instance.Get<ThreadScheduler>().Run();

    /* now we are in mainThread */

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_SEND_BLOCKED);

    Msg msg4 = Msg(instance);

    EXPECT_EQ(msg4.mSenderPid, KERNEL_PID_UNDEF);
    EXPECT_EQ(msg4.mType, 0);
    EXPECT_EQ(msg4.mContent.mPtr, nullptr);
    EXPECT_EQ(msg4.mContent.mValue, 0);

    msg4.Receive(); /* try to receive msg that was sent by task1Thread */

    EXPECT_EQ(msg4.mSenderPid, task1Thread->GetPid());
    EXPECT_EQ(msg4.mType, 0x21);
    EXPECT_EQ(msg4.mContent.mValue, 0xdeadbeef);

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_PENDING);

    /* Note: at this point msg3 already received by mainThread and because
     * task1Thread has higher priority than mainThread, kernel will context
     * switch to task1Thread */

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);
}

TEST_F(TestMsg, multipleSendReceiveMsg)
{
    // TODO
}
