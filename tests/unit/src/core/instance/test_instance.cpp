#include "gtest/gtest.h"

#include "core/instance.hpp"
#include "core/locator.hpp"
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

class InstanceLocatorTest : public InstanceLocator
{
public:
    InstanceLocatorTest(Instance &aInstance)
        : InstanceLocator(aInstance)
    {
    }
};

class InstanceLocatorInitTest : public InstanceLocatorInit
{
public:
    InstanceLocatorInitTest(void) {}

    void Init(Instance &aInstance) { InstanceLocatorInit::Init(aInstance); }
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

    InstanceLocatorTest instanceLocator1 = InstanceLocatorTest(*instance1);
    InstanceLocatorTest instanceLocator2 = InstanceLocatorTest(*instance2);

    EXPECT_EQ(&instanceLocator1.GetInstance(), instance1);
    EXPECT_EQ(&instanceLocator2.GetInstance(), instance2);

    InstanceLocatorInitTest instanceLocatorInit1 = InstanceLocatorInitTest();
    InstanceLocatorInitTest instanceLocatorInit2 = InstanceLocatorInitTest();

    instanceLocatorInit1.Init(*instance1);
    instanceLocatorInit2.Init(*instance2);

    EXPECT_EQ(&instanceLocatorInit1.GetInstance(), instance1);
    EXPECT_EQ(&instanceLocatorInit2.GetInstance(), instance2);
}
