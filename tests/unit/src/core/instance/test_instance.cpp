#include "gtest/gtest.h"

#include "core/instance.hpp"
#include "core/code_utils.hpp"

using namespace mt;

class TestInstance : public testing::Test
{
protected:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

TEST_F(TestInstance, init)
{
    mtDEFINE_ALIGNED_VAR(buffer1, sizeof(Instance), uint64_t);
    mtDEFINE_ALIGNED_VAR(buffer2, sizeof(Instance), uint64_t);

    uint32_t size1 = mtARRAY_LENGTH(buffer1);
    uint32_t size2 = mtARRAY_LENGTH(buffer2);

    Instance *instance1 = &Instance::Init((void *)buffer1, (size_t *)&size1);
    Instance *instance2 = &Instance::Init((void *)buffer2, (size_t *)&size2);

    EXPECT_TRUE(instance1->IsInitialized());
    EXPECT_TRUE(instance2->IsInitialized());
}
