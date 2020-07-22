#include "gtest/gtest.h"

#include "core/instance.hpp"
#include "core/code_utils.hpp"
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
    EXPECT_FALSE(instance.Get<ThreadScheduler>().IsContextSwitchRequestedFromIsr());
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

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] try send a message to a thread that doesn't in receive
     * blocked status
     * -------------------------------------------------------------------------
     **/

    Msg msg5 = Msg(instance);

    EXPECT_EQ(msg5.mSenderPid, KERNEL_PID_UNDEF);
    EXPECT_EQ(msg5.mType, 0);
    EXPECT_EQ(msg5.mContent.mPtr, nullptr);
    EXPECT_EQ(msg5.mContent.mValue, 0);

    msg5.mType = 0x22;
    msg5.mContent.mValue = 0xdeadbeef;

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);

    EXPECT_EQ(msg5.TrySend(mainThread->GetPid()), 0);

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);

    /* Note: none of these TrySend message will works unless the target thread
     * is in receive blocked status */

    EXPECT_EQ(msg5.TrySend(mainThread->GetPid()), 0);
    EXPECT_EQ(msg5.TrySend(mainThread->GetPid()), 0);
    EXPECT_EQ(msg5.TrySend(mainThread->GetPid()), 0);
    EXPECT_EQ(msg5.TrySend(mainThread->GetPid()), 0);
    EXPECT_EQ(msg5.TrySend(mainThread->GetPid()), 0);
    EXPECT_EQ(msg5.TrySend(mainThread->GetPid()), 0);

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);

    instance.Get<ThreadScheduler>().SleepingCurrentThread();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_SLEEPING);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_SLEEPING);

    Msg msg6 = Msg(instance);

    EXPECT_EQ(msg6.mSenderPid, KERNEL_PID_UNDEF);
    EXPECT_EQ(msg6.mType, 0);
    EXPECT_EQ(msg6.mContent.mPtr, nullptr);
    EXPECT_EQ(msg6.mContent.mValue, 0);

    msg6.Receive();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_SLEEPING);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_SLEEPING);

    instance.Get<ThreadScheduler>().WakeupThread(task1Thread->GetPid());

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);

    /* Note: now TrySend() will succeed */

    EXPECT_EQ(msg5.TrySend(mainThread->GetPid()), 1);

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);

    /* message is successfully received */

    EXPECT_EQ(msg6.mSenderPid, task1Thread->GetPid());
    EXPECT_EQ(msg6.mType, 0x22);
    EXPECT_EQ(msg6.mContent.mValue, 0xdeadbeef);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);

    EXPECT_EQ(msg5.SendToSelfQueue(), 0); /* we dont use message queue yet */

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] send message from Isr
     * -------------------------------------------------------------------------
     **/

    msg6.Receive();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RECEIVE_BLOCKED);

    testHelperSetCpuInIsr();

    msg5.mType = 0xff;
    msg5.mContent.mValue = 0x12345678;

    EXPECT_EQ(msg5.Send(task1Thread->GetPid()), 1);

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_PENDING);

    EXPECT_TRUE(instance.Get<ThreadScheduler>().IsContextSwitchRequestedFromIsr());

    mtCpuEndOfIsr((mtInstance *)&instance);

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_PENDING);

    testHelperResetCpuInIsr();

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);

    EXPECT_EQ(msg6.mSenderPid, KERNEL_PID_ISR);
    EXPECT_EQ(msg6.mType, 0xff);
    EXPECT_EQ(msg6.mContent.mValue, 0x12345678);

    /* Note: send message from Isr when the target is not in receive blocked */

    testHelperSetCpuInIsr();

    EXPECT_EQ(msg5.Send(task1Thread->GetPid()), 0);

    mtCpuEndOfIsr((mtInstance *)&instance);

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);

    testHelperResetCpuInIsr();

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] send and then immediately set to receive state by calling
     * sendReceive function.
     * -------------------------------------------------------------------------
     **/

    Msg msg7 = Msg(instance);
    Msg msg7Reply = Msg(instance);

    EXPECT_EQ(msg7.mSenderPid, KERNEL_PID_UNDEF);
    EXPECT_EQ(msg7.mType, 0);
    EXPECT_EQ(msg7.mContent.mPtr, nullptr);
    EXPECT_EQ(msg7.mContent.mValue, 0);

    EXPECT_EQ(msg7Reply.mSenderPid, KERNEL_PID_UNDEF);
    EXPECT_EQ(msg7Reply.mType, 0);
    EXPECT_EQ(msg7Reply.mContent.mPtr, nullptr);
    EXPECT_EQ(msg7Reply.mContent.mValue, 0);

    uint32_t msg7Data = 0xdeeeaaad;

    msg7.mType = 0xfe;
    msg7.mContent.mPtr = static_cast<void *>(&msg7Data);

    EXPECT_EQ(msg7.SendReceive(&msg7Reply, mainThread->GetPid()), 1);

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_REPLY_BLOCKED);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_REPLY_BLOCKED);

    Msg msg8 = Msg(instance);
    Msg msg8Reply = Msg(instance);

    EXPECT_EQ(msg8.mSenderPid, KERNEL_PID_UNDEF);
    EXPECT_EQ(msg8.mType, 0);
    EXPECT_EQ(msg8.mContent.mPtr, nullptr);
    EXPECT_EQ(msg8.mContent.mValue, 0);

    EXPECT_EQ(msg8Reply.mSenderPid, KERNEL_PID_UNDEF);
    EXPECT_EQ(msg8Reply.mType, 0);
    EXPECT_EQ(msg8Reply.mContent.mPtr, nullptr);
    EXPECT_EQ(msg8Reply.mContent.mValue, 0);

    msg8.Receive();

    EXPECT_EQ(msg8.mSenderPid, task1Thread->GetPid());
    EXPECT_EQ(msg8.mType, 0xfe);
    EXPECT_EQ(*static_cast<uint32_t *>(msg8.mContent.mPtr), 0xdeeeaaad);

    /* Note: at this point msg7 is received */

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_REPLY_BLOCKED);

    msg8Reply.mType = 0xff;
    msg8Reply.mContent.mValue = 0xdeadbeef;

    EXPECT_EQ(msg8.Reply(&msg8Reply), 1);

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_PENDING);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);

    EXPECT_EQ(msg7Reply.mType, 0xff);
    EXPECT_EQ(msg7Reply.mContent.mValue, 0xdeadbeef);

    /* Note: reply msg does not care who was replying the message */

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] send and then immediately set to receive state by calling
     * sendReceive function. Reply function would be in Isr.
     * -------------------------------------------------------------------------
     **/

    msg7.mType = 0xab;
    msg7.mContent.mValue = 0xaaaabbbb;

    EXPECT_EQ(msg7.SendReceive(&msg7Reply, mainThread->GetPid()), 1);

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_REPLY_BLOCKED);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_REPLY_BLOCKED);

    msg8.Receive();

    EXPECT_EQ(msg8.mSenderPid, task1Thread->GetPid());
    EXPECT_EQ(msg8.mType, 0xab);
    EXPECT_EQ(msg8.mContent.mValue, 0xaaaabbbb);

    /* Note: at this point msg7 is received */

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_REPLY_BLOCKED);

    msg8Reply.mType = 0xcd;
    msg8Reply.mContent.mValue = 0xccccdddd;

    testHelperSetCpuInIsr(); /* ----------------- set cpu artificially in Isr */

    EXPECT_EQ(msg8.ReplyInIsr(&msg8Reply), 1);

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_PENDING);

    mtCpuEndOfIsr(&instance);

    testHelperResetCpuInIsr(); /* ---------------------------------- exit Isr */

    EXPECT_TRUE(instance.Get<ThreadScheduler>().IsContextSwitchRequestedFromIsr());

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);

    EXPECT_EQ(msg7Reply.mType, 0xcd);
    EXPECT_EQ(msg7Reply.mContent.mValue, 0xccccdddd);

    /* Note: reply message was sent from Isr */
}

TEST_F(TestMsg, multipleSendReceiveMsg)
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
    EXPECT_FALSE(instance.Get<ThreadScheduler>().IsContextSwitchRequestedFromIsr());
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
     * [TEST CASE] create msg queue in current active thread
     * -------------------------------------------------------------------------
     **/

    EXPECT_EQ(task1Thread->HasMsgQueue(), 0);

    Msg task1MsgArray[16];

    for (int i = 0; i < 16; i++)
    {
        task1MsgArray[i].Init(instance);
    }

    task1Thread->InitMsgQueue(task1MsgArray, mtARRAY_LENGTH(task1MsgArray));

    EXPECT_EQ(task1Thread->HasMsgQueue(), 1);
    EXPECT_EQ(task1Thread->GetNumOfMsgInQueue(), 0);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] set current active thread to sleep status and send msg to it,
     * it expected to queued the msg.
     * -------------------------------------------------------------------------
     **/

    instance.Get<ThreadScheduler>().SetThreadStatusAndUpdateRunqueue(task1Thread, THREAD_STATUS_SLEEPING);

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_SLEEPING);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_SLEEPING);

    Msg msg1 = Msg(instance);
    Msg msg2 = Msg(instance);
    Msg msg3 = Msg(instance);
    Msg msg4 = Msg(instance);

    msg1.mType = 0xff;
    msg1.mContent.mValue = 0x1;

    msg2.mType = 0xff;
    msg2.mContent.mValue = 0x2;

    msg3.mType = 0xff;
    msg3.mContent.mValue = 0x3;

    msg4.mType = 0xff;
    msg4.mContent.mValue = 0x4;

    EXPECT_EQ(msg1.Send(task1Thread->GetPid()), 1);
    EXPECT_EQ(msg2.Send(task1Thread->GetPid()), 1);
    EXPECT_EQ(msg3.Send(task1Thread->GetPid()), 1);
    EXPECT_EQ(msg4.Send(task1Thread->GetPid()), 1);

    /* Note: task1Thread is in sleeping status and has message queue, so the
     * message sent to task1Thread should be queued */

    EXPECT_EQ(task1Thread->GetNumOfMsgInQueue(), 4);

    /* Note: set task1Thread to running status and dequeque the message */

    instance.Get<ThreadScheduler>().SetThreadStatusAndUpdateRunqueue(task1Thread, THREAD_STATUS_PENDING);

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_PENDING);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);

    EXPECT_EQ(task1Thread->GetNumOfMsgInQueue(), 4);

    Msg msgInTask1 = Msg(instance);

    EXPECT_EQ(msgInTask1.Receive(), 1);

    EXPECT_EQ(msgInTask1.mSenderPid, mainThread->GetPid());
    EXPECT_EQ(msgInTask1.mType, 0xff);
    EXPECT_EQ(msgInTask1.mContent.mValue, 0x01); /* get msg1 */

    EXPECT_EQ(msgInTask1.Receive(), 1);

    EXPECT_EQ(msgInTask1.mSenderPid, mainThread->GetPid());
    EXPECT_EQ(msgInTask1.mType, 0xff);
    EXPECT_EQ(msgInTask1.mContent.mValue, 0x02); /* get msg2 */

     EXPECT_EQ(msgInTask1.Receive(), 1);

    EXPECT_EQ(msgInTask1.mSenderPid, mainThread->GetPid());
    EXPECT_EQ(msgInTask1.mType, 0xff);
    EXPECT_EQ(msgInTask1.mContent.mValue, 0x03); /* get msg3 */

    EXPECT_EQ(msgInTask1.Receive(), 1);

    EXPECT_EQ(msgInTask1.mSenderPid, mainThread->GetPid());
    EXPECT_EQ(msgInTask1.mType, 0xff);
    EXPECT_EQ(msgInTask1.mContent.mValue, 0x04); /* get msg4 */

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);

    EXPECT_EQ(task1Thread->GetNumOfMsgInQueue(), 0);

    /* Note: at this point we already got all the message from queue */

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] send message to itself
     * -------------------------------------------------------------------------
     **/

    EXPECT_EQ(msg1.Send(task1Thread->GetPid()), 1);
    EXPECT_EQ(msg2.TrySend(task1Thread->GetPid()), 1);
    EXPECT_EQ(msg3.Send(task1Thread->GetPid()), 1);
    EXPECT_EQ(msg4.TrySend(task1Thread->GetPid()), 1);

    EXPECT_EQ(task1Thread->GetNumOfMsgInQueue(), 4);

    EXPECT_EQ(msgInTask1.Receive(), 1);

    EXPECT_EQ(msgInTask1.mSenderPid, task1Thread->GetPid());
    EXPECT_EQ(msgInTask1.mType, 0xff);
    EXPECT_EQ(msgInTask1.mContent.mValue, 0x01); /* get msg1 */

    EXPECT_EQ(msgInTask1.Receive(), 1);

    EXPECT_EQ(msgInTask1.mSenderPid, task1Thread->GetPid());
    EXPECT_EQ(msgInTask1.mType, 0xff);
    EXPECT_EQ(msgInTask1.mContent.mValue, 0x02); /* get msg2 */

     EXPECT_EQ(msgInTask1.Receive(), 1);

    EXPECT_EQ(msgInTask1.mSenderPid, task1Thread->GetPid());
    EXPECT_EQ(msgInTask1.mType, 0xff);
    EXPECT_EQ(msgInTask1.mContent.mValue, 0x03); /* get msg3 */

    EXPECT_EQ(msgInTask1.Receive(), 1);

    EXPECT_EQ(msgInTask1.mSenderPid, task1Thread->GetPid());
    EXPECT_EQ(msgInTask1.mType, 0xff);
    EXPECT_EQ(msgInTask1.mContent.mValue, 0x04); /* get msg4 */

    EXPECT_EQ(mainThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idleThread->GetStatus(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1Thread->GetStatus(), THREAD_STATUS_RUNNING);

    EXPECT_EQ(task1Thread->GetNumOfMsgInQueue(), 0);
}
