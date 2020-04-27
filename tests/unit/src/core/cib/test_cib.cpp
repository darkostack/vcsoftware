#include "gtest/gtest.h"

#include "cib.hpp"

using namespace mt;

class TestCib : public testing::Test
{
protected:
    Cib *obj;

    virtual void SetUp()
    {
        obj = new Cib(4);
    }

    virtual void TearDown()
    {
        delete obj;
    }
};

TEST_F(TestCib, constructor)
{
    EXPECT_TRUE(obj);
}

TEST_F(TestCib, functions)
{
    EXPECT_EQ(obj->Full(), 0);
    EXPECT_EQ(obj->Avail(), 0);
    EXPECT_EQ(obj->GetReadCount(), 0);
    EXPECT_EQ(obj->GetWriteCount(), 0);

    EXPECT_EQ(obj->GetMask(), 3);

    EXPECT_EQ(obj->Put(), 0);
    EXPECT_EQ(obj->Put(), 1);
    EXPECT_EQ(obj->Put(), 2);
    EXPECT_EQ(obj->Put(), 3);

    EXPECT_EQ(obj->Put(), -1); /* cib already full */
    EXPECT_EQ(obj->Full(), 1);

    EXPECT_EQ(obj->PutUnsafe(), 0); /* at this point cib was overwrite */

    EXPECT_EQ(obj->GetWriteCount(), 5);

    obj->Init(4);

    EXPECT_EQ(obj->Full(), 0);
    EXPECT_EQ(obj->Avail(), 0);
    EXPECT_EQ(obj->GetReadCount(), 0);
    EXPECT_EQ(obj->GetWriteCount(), 0);

    EXPECT_EQ(obj->GetMask(), 3);

    EXPECT_EQ(obj->Put(), 0);
    EXPECT_EQ(obj->Put(), 1);
    EXPECT_EQ(obj->Put(), 2);
    EXPECT_EQ(obj->Put(), 3);

    EXPECT_EQ(obj->Peek(), 0);

    EXPECT_EQ(obj->Full(), 1);
    EXPECT_EQ(obj->Avail(), 4);
    EXPECT_EQ(obj->GetReadCount(), 0);
    EXPECT_EQ(obj->GetWriteCount(), 4);

    EXPECT_EQ(obj->Get(), 0);
    EXPECT_EQ(obj->Get(), 1);

    EXPECT_EQ(obj->Peek(), 2);

    EXPECT_EQ(obj->Full(), 0);
    EXPECT_EQ(obj->Avail(), 2);
    EXPECT_EQ(obj->GetReadCount(), 2);
    EXPECT_EQ(obj->GetWriteCount(), 4);

    EXPECT_EQ(obj->Get(), 2);
    EXPECT_EQ(obj->Get(), 3);

    EXPECT_EQ(obj->Peek(), -1);

    EXPECT_EQ(obj->Full(), 0);
    EXPECT_EQ(obj->Avail(), 0);
    EXPECT_EQ(obj->GetReadCount(), 4);
    EXPECT_EQ(obj->GetWriteCount(), 4);
}
