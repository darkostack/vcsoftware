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


