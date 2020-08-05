#include "gtest/gtest.h"

#include "core/instance.hpp"
#include "core/code_utils.hpp"

using namespace vc;

DEFINE_ALIGNED_VAR(buffer1, sizeof(Instance), uint64_t);
uint32_t buffer1_size = ARRAY_LENGTH(buffer1);

DEFINE_ALIGNED_VAR(buffer2, sizeof(Instance), uint64_t);
uint32_t buffer2_size = ARRAY_LENGTH(buffer2);

class TestInstance : public testing::Test
{
protected:
    Instance *instance1;
    Instance *instance2;

    virtual void SetUp()
    {
        instance1 = &Instance::init((void *)buffer1, (size_t *)&buffer1_size);
        instance2 = &Instance::init((void *)buffer2, (size_t *)&buffer2_size);
    }
};

TEST_F(TestInstance, constructor_test)
{
    EXPECT_TRUE(instance1);
    EXPECT_TRUE(instance2);
}

TEST_F(TestInstance, init_test)
{
    EXPECT_TRUE(instance1->is_initialized());
    EXPECT_TRUE(instance2->is_initialized());
}
