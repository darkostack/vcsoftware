#include "gtest/gtest.h"

#include "core/clist.hpp"

using namespace mt;

class TestClist : public testing::Test
{
protected:
    Clist *obj;

    virtual void SetUp()
    {
        obj = new Clist;
    }

    virtual void TearDown()
    {
        delete obj;
    }
};

TEST_F(TestClist, constructor)
{
    EXPECT_TRUE(obj);
}

TEST_F(TestClist, right_push)
{
    Clist node1;
    Clist node2;
    Clist node3;
    Clist node4;

    obj->RightPush(&node1);

    /* obj->1->1 */

    EXPECT_EQ(obj->mNext, &node1);
    EXPECT_EQ(obj->mNext->mNext, &node1);

    obj->RightPush(&node2);

    /* obj->2->1->2 */

    EXPECT_EQ(obj->mNext, &node2);
    EXPECT_EQ(obj->mNext->mNext, &node1);
    EXPECT_EQ(obj->mNext->mNext->mNext, &node2);

    obj->RightPush(&node3);

    /* obj->3->1->2->3 */

    EXPECT_EQ(obj->mNext, &node3);
    EXPECT_EQ(obj->mNext->mNext, &node1);
    EXPECT_EQ(obj->mNext->mNext->mNext, &node2);
    EXPECT_EQ(obj->mNext->mNext->mNext->mNext, &node3);

    obj->RightPush(&node4);

    /* obj->4->1->2->3->4 */

    EXPECT_EQ(obj->mNext, &node4);
    EXPECT_EQ(obj->mNext->mNext, &node1);
    EXPECT_EQ(obj->mNext->mNext->mNext, &node2);
    EXPECT_EQ(obj->mNext->mNext->mNext->mNext, &node3);
    EXPECT_EQ(obj->mNext->mNext->mNext->mNext->mNext, &node4);
}

TEST_F(TestClist, left_push)
{
    Clist node1;
    Clist node2;
    Clist node3;
    Clist node4;

    obj->LeftPush(&node1);

    /* obj->1->1 */

    EXPECT_EQ(obj->mNext, &node1);
    EXPECT_EQ(obj->mNext->mNext, &node1);

    obj->LeftPush(&node2);

    /* obj->1->2->1 */

    EXPECT_EQ(obj->mNext, &node1);
    EXPECT_EQ(obj->mNext->mNext, &node2);
    EXPECT_EQ(obj->mNext->mNext->mNext, &node1);

    obj->LeftPush(&node3);

    /* obj->1->3->2->1 */

    EXPECT_EQ(obj->mNext, &node1);
    EXPECT_EQ(obj->mNext->mNext, &node3);
    EXPECT_EQ(obj->mNext->mNext->mNext, &node2);
    EXPECT_EQ(obj->mNext->mNext->mNext->mNext, &node1);

    obj->LeftPush(&node4);

    /* obj->1->4->3->2->1 */

    EXPECT_EQ(obj->mNext, &node1);
    EXPECT_EQ(obj->mNext->mNext, &node4);
    EXPECT_EQ(obj->mNext->mNext->mNext, &node3);
    EXPECT_EQ(obj->mNext->mNext->mNext->mNext, &node2);
    EXPECT_EQ(obj->mNext->mNext->mNext->mNext->mNext, &node1);
}

TEST_F(TestClist, left_pop)
{
    Clist node1;
    Clist node2;
    Clist node3;
    Clist node4;

    obj->LeftPush(&node1);
    obj->LeftPush(&node2);
    obj->LeftPush(&node3);
    obj->LeftPush(&node4);

    /* obj->1->4->3->2->1 */

    EXPECT_EQ(obj->mNext, &node1);
    EXPECT_EQ(obj->mNext->mNext, &node4);
    EXPECT_EQ(obj->mNext->mNext->mNext, &node3);
    EXPECT_EQ(obj->mNext->mNext->mNext->mNext, &node2);
    EXPECT_EQ(obj->mNext->mNext->mNext->mNext->mNext, &node1);

    EXPECT_EQ(obj->LeftPop(), &node4);

    /* obj->1->3->2->1 */

    EXPECT_EQ(obj->mNext, &node1);
    EXPECT_EQ(obj->mNext->mNext, &node3);
    EXPECT_EQ(obj->mNext->mNext->mNext, &node2);
    EXPECT_EQ(obj->mNext->mNext->mNext->mNext, &node1);

    EXPECT_EQ(obj->LeftPop(), &node3);
    EXPECT_EQ(obj->LeftPop(), &node2);

    /* obj->1->1 */

    EXPECT_EQ(obj->mNext, &node1);
    EXPECT_EQ(obj->mNext->mNext, &node1);

    EXPECT_EQ(obj->LeftPop(), &node1);

    /* obj->null */

    EXPECT_EQ(obj->mNext, nullptr);
}

TEST_F(TestClist, left_pop_right_push)
{
    Clist node1;
    Clist node2;
    Clist node3;
    Clist node4;

    obj->LeftPush(&node1);
    obj->LeftPush(&node2);
    obj->LeftPush(&node3);
    obj->LeftPush(&node4);

    /* obj->1->4->3->2->1 */

    EXPECT_EQ(obj->mNext, &node1);
    EXPECT_EQ(obj->mNext->mNext, &node4);
    EXPECT_EQ(obj->mNext->mNext->mNext, &node3);
    EXPECT_EQ(obj->mNext->mNext->mNext->mNext, &node2);
    EXPECT_EQ(obj->mNext->mNext->mNext->mNext->mNext, &node1);

    obj->LeftPopRightPush();

    /* obj->4->3->2->1->4 */

    EXPECT_EQ(obj->mNext, &node4);
    EXPECT_EQ(obj->mNext->mNext, &node3);
    EXPECT_EQ(obj->mNext->mNext->mNext, &node2);
    EXPECT_EQ(obj->mNext->mNext->mNext->mNext, &node1);
    EXPECT_EQ(obj->mNext->mNext->mNext->mNext->mNext, &node4);

    obj->LeftPopRightPush();

    /* obj->3->2->1->4->3 */

    EXPECT_EQ(obj->mNext, &node3);
    EXPECT_EQ(obj->mNext->mNext, &node2);
    EXPECT_EQ(obj->mNext->mNext->mNext, &node1);
    EXPECT_EQ(obj->mNext->mNext->mNext->mNext, &node4);
    EXPECT_EQ(obj->mNext->mNext->mNext->mNext->mNext, &node3);

    /* obj->2->1->4->3->2 */

    obj->LeftPopRightPush();

    EXPECT_EQ(obj->mNext, &node2);
    EXPECT_EQ(obj->mNext->mNext, &node1);
    EXPECT_EQ(obj->mNext->mNext->mNext, &node4);
    EXPECT_EQ(obj->mNext->mNext->mNext->mNext, &node3);
    EXPECT_EQ(obj->mNext->mNext->mNext->mNext->mNext, &node2);

    obj->LeftPopRightPush();

    /* obj->1->4->3->2->1 */

    EXPECT_EQ(obj->mNext, &node1);
    EXPECT_EQ(obj->mNext->mNext, &node4);
    EXPECT_EQ(obj->mNext->mNext->mNext, &node3);
    EXPECT_EQ(obj->mNext->mNext->mNext->mNext, &node2);
    EXPECT_EQ(obj->mNext->mNext->mNext->mNext->mNext, &node1);
}

TEST_F(TestClist, count)
{
    Clist node1;
    Clist node2;
    Clist node3;
    Clist node4;

    obj->LeftPush(&node1);
    obj->LeftPush(&node2);
    obj->LeftPush(&node3);
    obj->LeftPush(&node4);

    /* obj->1->4->3->2->1 */

    EXPECT_EQ(obj->mNext, &node1);
    EXPECT_EQ(obj->mNext->mNext, &node4);
    EXPECT_EQ(obj->mNext->mNext->mNext, &node3);
    EXPECT_EQ(obj->mNext->mNext->mNext->mNext, &node2);
    EXPECT_EQ(obj->mNext->mNext->mNext->mNext->mNext, &node1);

    EXPECT_EQ(obj->Count(), 4);
}

TEST_F(TestClist, remove)
{
    Clist node1;
    Clist node2;
    Clist node3;
    Clist node4;

    obj->LeftPush(&node1);
    obj->LeftPush(&node2);
    obj->LeftPush(&node3);
    obj->LeftPush(&node4);

    /* obj->1->4->3->2->1 */

    EXPECT_EQ(obj->mNext, &node1);
    EXPECT_EQ(obj->mNext->mNext, &node4);
    EXPECT_EQ(obj->mNext->mNext->mNext, &node3);
    EXPECT_EQ(obj->mNext->mNext->mNext->mNext, &node2);
    EXPECT_EQ(obj->mNext->mNext->mNext->mNext->mNext, &node1);

    EXPECT_EQ(obj->Remove(&node3), &node3);

    /* obj->1->4->2->1 */

    EXPECT_EQ(obj->mNext, &node1);
    EXPECT_EQ(obj->mNext->mNext, &node4);
    EXPECT_EQ(obj->mNext->mNext->mNext, &node2);
    EXPECT_EQ(obj->mNext->mNext->mNext->mNext, &node1);

    EXPECT_EQ(obj->Remove(&node1), &node1);

    /* obj->2->4->2 */

    EXPECT_EQ(obj->mNext, &node2);
    EXPECT_EQ(obj->mNext->mNext, &node4);
    EXPECT_EQ(obj->mNext->mNext->mNext, &node2);

    EXPECT_EQ(obj->Remove(&node4), &node4);

    /* obj->2->2 */

    EXPECT_EQ(obj->mNext, &node2);
    EXPECT_EQ(obj->mNext->mNext, &node2);

    EXPECT_EQ(obj->Remove(&node2), &node2);

    /* obj->null */

    EXPECT_EQ(obj->mNext, nullptr);
}

TEST_F(TestClist, find)
{
    Clist node1;
    Clist node2;
    Clist node3;
    Clist node4;

    obj->LeftPush(&node1);
    obj->LeftPush(&node2);
    obj->LeftPush(&node3);
    obj->LeftPush(&node4);

    /* obj->1->4->3->2->1 */

    EXPECT_EQ(obj->mNext, &node1);
    EXPECT_EQ(obj->mNext->mNext, &node4);
    EXPECT_EQ(obj->mNext->mNext->mNext, &node3);
    EXPECT_EQ(obj->mNext->mNext->mNext->mNext, &node2);
    EXPECT_EQ(obj->mNext->mNext->mNext->mNext->mNext, &node1);

    EXPECT_EQ(obj->Find(&node1), &node1);
    EXPECT_EQ(obj->Find(&node3), &node3);
    EXPECT_EQ(obj->Find(&node2), &node2);
    EXPECT_EQ(obj->Find(&node4), &node4);

    EXPECT_EQ(obj->Remove(&node3), &node3);

    EXPECT_EQ(obj->Find(&node3), nullptr);

    /* obj->1->4->2->1 */

    EXPECT_EQ(obj->mNext, &node1);
    EXPECT_EQ(obj->mNext->mNext, &node4);
    EXPECT_EQ(obj->mNext->mNext->mNext, &node2);
    EXPECT_EQ(obj->mNext->mNext->mNext->mNext, &node1);
}

TEST_F(TestClist, find_before)
{
    Clist node1;
    Clist node2;
    Clist node3;
    Clist node4;

    obj->LeftPush(&node1);
    obj->LeftPush(&node2);
    obj->LeftPush(&node3);
    obj->LeftPush(&node4);

    /* obj->1->4->3->2->1 */

    EXPECT_EQ(obj->mNext, &node1);
    EXPECT_EQ(obj->mNext->mNext, &node4);
    EXPECT_EQ(obj->mNext->mNext->mNext, &node3);
    EXPECT_EQ(obj->mNext->mNext->mNext->mNext, &node2);
    EXPECT_EQ(obj->mNext->mNext->mNext->mNext->mNext, &node1);

    EXPECT_EQ(obj->FindBefore(&node3), &node4);
    EXPECT_EQ(obj->FindBefore(&node4), &node1);
    EXPECT_EQ(obj->FindBefore(&node1), &node2);
    EXPECT_EQ(obj->FindBefore(&node2), &node3);
}

TEST_F(TestClist, right_pop)
{
    Clist node1;
    Clist node2;
    Clist node3;
    Clist node4;

    obj->LeftPush(&node1);
    obj->LeftPush(&node2);
    obj->LeftPush(&node3);
    obj->LeftPush(&node4);

    /* obj->1->4->3->2->1 */

    EXPECT_EQ(obj->mNext, &node1);
    EXPECT_EQ(obj->mNext->mNext, &node4);
    EXPECT_EQ(obj->mNext->mNext->mNext, &node3);
    EXPECT_EQ(obj->mNext->mNext->mNext->mNext, &node2);
    EXPECT_EQ(obj->mNext->mNext->mNext->mNext->mNext, &node1);

    EXPECT_EQ(obj->RightPop(), &node1);

    /* obj->2->4->3->2 */

    EXPECT_EQ(obj->mNext, &node2);
    EXPECT_EQ(obj->mNext->mNext, &node4);
    EXPECT_EQ(obj->mNext->mNext->mNext, &node3);
    EXPECT_EQ(obj->mNext->mNext->mNext->mNext, &node2);

    EXPECT_EQ(obj->RightPop(), &node2);

    /* obj->3->4->3 */

    EXPECT_EQ(obj->mNext, &node3);
    EXPECT_EQ(obj->mNext->mNext, &node4);
    EXPECT_EQ(obj->mNext->mNext->mNext, &node3);

    EXPECT_EQ(obj->RightPop(), &node3);

    /* obj->4->4 */

    EXPECT_EQ(obj->mNext, &node4);
    EXPECT_EQ(obj->mNext->mNext, &node4);

    EXPECT_EQ(obj->RightPop(), &node4);

    /* obj->null */

    EXPECT_EQ(obj->mNext, nullptr);
}

TEST_F(TestClist, left_right_peek)
{
    Clist node1;
    Clist node2;
    Clist node3;
    Clist node4;

    obj->LeftPush(&node1);
    obj->LeftPush(&node2);
    obj->LeftPush(&node3);
    obj->LeftPush(&node4);

    /* obj->1->4->3->2->1 */

    EXPECT_EQ(obj->mNext, &node1);
    EXPECT_EQ(obj->mNext->mNext, &node4);
    EXPECT_EQ(obj->mNext->mNext->mNext, &node3);
    EXPECT_EQ(obj->mNext->mNext->mNext->mNext, &node2);
    EXPECT_EQ(obj->mNext->mNext->mNext->mNext->mNext, &node1);

    EXPECT_EQ(obj->LeftPeek(), &node4);
    EXPECT_EQ(obj->RightPeek(), &node1);

    /* obj->1->3->2->1 */

    EXPECT_EQ(obj->LeftPop(), &node4);

    EXPECT_EQ(obj->LeftPeek(), &node3);
    EXPECT_EQ(obj->RightPeek(), &node1);

    /* obj->2->3->2 */

    EXPECT_EQ(obj->RightPop(), &node1);

    EXPECT_EQ(obj->mNext, &node2);
    EXPECT_EQ(obj->mNext->mNext, &node3);
    EXPECT_EQ(obj->mNext->mNext->mNext, &node2);

    EXPECT_EQ(obj->LeftPeek(), &node3);
    EXPECT_EQ(obj->RightPeek(), &node2);

    EXPECT_EQ(obj->LeftPop(), &node3);

    /* obj->2->2 */

    EXPECT_EQ(obj->mNext, &node2);
    EXPECT_EQ(obj->mNext->mNext, &node2);

    EXPECT_EQ(obj->LeftPeek(), &node2);
    EXPECT_EQ(obj->RightPeek(), &node2);

    EXPECT_EQ(obj->RightPop(), &node2);

    /* obj->null */

    EXPECT_EQ(obj->mNext, nullptr);

    EXPECT_EQ(obj->LeftPeek(), nullptr);
    EXPECT_EQ(obj->RightPeek(), nullptr);
}
