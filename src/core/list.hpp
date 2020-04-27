#ifndef CORE_LIST_HPP
#define CORE_LIST_HPP

#include <mtos/list.h>

namespace mt {

class List : public mtListNode
{
public:
    explicit List(void) { mNext = NULL; }

    void Add(List *aNode)
    {
        aNode->mNext = this->mNext;
        this->mNext = aNode;
    }

    List *RemoveHead(void)
    {
        List *head = static_cast<List *>(this->mNext);

        if (head)
        {
            this->mNext = head->mNext;
        }

        return head;
    }

    static List *Remove(List *aList, List *aNode)
    {
        while (aList->mNext)
        {
            if (aList->mNext == aNode)
            {
                aList->mNext = aNode->mNext;
                return aNode;
            }
            aList = static_cast<List *>(aList->mNext);
        }

        return static_cast<List *>(aList->mNext);
    }
};

} // namespace mt

#endif /* CORE_LIST_HPP */
