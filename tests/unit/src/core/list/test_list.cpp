#include "gtest/gtest.h"

#include "core/list.hpp"

using namespace mt;

class TestList : public testing::Test
{
protected:
    List *obj;

    virtual void SetUp()
    {
        obj = new List;
    }

    virtual void TearDown()
    {
        delete obj;
    }
};

TEST_F(TestList, constructor)
{
    EXPECT_TRUE(obj);
}

TEST_F(TestList, functions)
{
    List node1;
    List node2;
    List node3;

    obj->Add(&node1);
    obj->Add(&node2);
    obj->Add(&node3);

    /* obj->3->2->1->null */

    EXPECT_EQ(obj->mNext, &node3);
    EXPECT_EQ(obj->mNext->mNext, &node2);
    EXPECT_EQ(obj->mNext->mNext->mNext, &node1);
    EXPECT_EQ(obj->mNext->mNext->mNext->mNext, nullptr);

    EXPECT_EQ(obj->RemoveHead(), &node3);

    /* obj->2->1->null */

    EXPECT_EQ(obj->mNext, &node2);
    EXPECT_EQ(obj->mNext->mNext, &node1);
    EXPECT_EQ(obj->mNext->mNext->mNext, nullptr);

    obj->Add(&node3);

    /* obj->3->2->1->null */

    EXPECT_EQ(obj->mNext, &node3);
    EXPECT_EQ(obj->mNext->mNext, &node2);
    EXPECT_EQ(obj->mNext->mNext->mNext, &node1);
    EXPECT_EQ(obj->mNext->mNext->mNext->mNext, nullptr);

    EXPECT_EQ(List::Remove(obj, &node2), &node2);

    /* obj->3->1->null */

    EXPECT_EQ(obj->mNext, &node3);
    EXPECT_EQ(obj->mNext->mNext, &node1);
    EXPECT_EQ(obj->mNext->mNext->mNext, nullptr);

    EXPECT_EQ(List::Remove(obj, &node3), &node3);

    /* obj->1->null */

    EXPECT_EQ(obj->mNext, &node1);
    EXPECT_EQ(obj->mNext->mNext, nullptr);

    EXPECT_EQ(List::Remove(obj, &node1), &node1);

    /* obj->null */

    EXPECT_EQ(obj->mNext, nullptr);
}
