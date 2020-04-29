#include "gtest/gtest.h"

#include "core/instance.hpp"
#include "core/code_utils.hpp"
#include "core/locator-getters.hpp"

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

    char stack[128];

    Thread *thread = Thread::Init(instance,
                                  stack,
                                  sizeof(stack),
                                  15,
                                  THREAD_FLAGS_CREATE_WOUT_YIELD_OTHER_THREAD | THREAD_FLAGS_CREATE_STACKMARKER,
                                  NULL,
                                  NULL,
                                  "idle");

    EXPECT_NE(thread, nullptr);

    EXPECT_EQ(thread->GetPid(), 1);
    EXPECT_EQ(thread->GetPriority(), 15);
    EXPECT_EQ(thread->GetName(), "idle");
    EXPECT_EQ(thread->GetStatus(), THREAD_STATUS_PENDING);

    EXPECT_EQ(instance.Get<ThreadScheduler>().GetNumOfThreadsInScheduler(), 1);
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetThreadFromScheduler(thread->GetPid()), thread);
    EXPECT_EQ(instance.Get<ThreadScheduler>().IsContextSwitchRequested(), 0);
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActiveThread(), nullptr);
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActivePid(), KERNEL_PID_UNDEF);

    instance.Get<ThreadScheduler>().Run();

    EXPECT_EQ(thread->GetStatus(), THREAD_STATUS_RUNNING);

    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActiveThread(), thread);
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetCurrentActivePid(), thread->GetPid());
    EXPECT_EQ(instance.Get<ThreadScheduler>().GetNumOfThreadsInScheduler(), 1);
}
