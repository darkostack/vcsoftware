#ifndef CORE_CLIST_HPP
#define CORE_CLIST_HPP

#include <stddef.h>

#include <mtos/clist.h>

#include "core/list.hpp"

namespace mt {

class Clist : public List
{
public:
    Clist(void) { mNext = NULL; }

    void RightPush(Clist *aNode)
    {
        if (this->mNext)
        {
            aNode->mNext = this->mNext->mNext;
            this->mNext->mNext = aNode;
        }
        else
        {
            aNode->mNext = aNode;
        }

        this->mNext = aNode;
    }

    void LeftPush(Clist *aNode)
    {
        if (this->mNext)
        {
            aNode->mNext = this->mNext->mNext;
            this->mNext->mNext = aNode;
        }
        else
        {
            aNode->mNext = aNode;
            this->mNext = aNode;
        }
    }

    Clist *LeftPop(void)
    {
        if (this->mNext)
        {
            Clist *first = static_cast<Clist *>(this->mNext->mNext);

            if (this->mNext == first)
            {
                this->mNext = NULL;
            }
            else
            {
                this->mNext->mNext = first->mNext;
            }

            return first;
        }
        else
        {
            return NULL;
        }
    }

    void LeftPopRightPush(void)
    {
        if (this->mNext)
        {
            this->mNext = this->mNext->mNext;
        }
    }

    Clist *LeftPeek(void)
    {
        if (this->mNext)
        {
            return static_cast<Clist *>(this->mNext->mNext);
        }

        return NULL;
    }

    Clist *RightPeek(void) { return static_cast<Clist *>(this->mNext); }

    Clist *RightPop(void)
    {
        if (this->mNext)
        {
            List *last = static_cast<List *>(this->mNext);

            while (this->mNext->mNext != last)
            {
                this->LeftPopRightPush();
            }

            return this->LeftPop();
        }
        else
        {
            return NULL;
        }
    }

    Clist *FindBefore(const Clist *aNode)
    {
        Clist *pos = static_cast<Clist *>(this->mNext);

        if (!pos)
        {
            return NULL;
        }

        do
        {
            pos = static_cast<Clist *>(pos->mNext);

            if (pos->mNext == aNode)
            {
                return pos;
            }

        } while (pos != this->mNext);

        return NULL;
    }

    Clist *Find(const Clist *aNode)
    {
        Clist *tmp = this->FindBefore(aNode);

        if (tmp)
        {
            return static_cast<Clist *>(tmp->mNext);
        }
        else
        {
            return NULL;
        }
    }

    Clist *Remove(Clist *aNode)
    {
        if (this->mNext)
        {
            if (this->mNext->mNext == aNode)
            {
                return this->LeftPop();
            }
            else
            {
                Clist *tmp = this->FindBefore(aNode);

                if (tmp)
                {
                    tmp->mNext = tmp->mNext->mNext;

                    if (aNode == this->mNext)
                    {
                        this->mNext = tmp;
                    }
                    return aNode;
                }
            }
        }

        return NULL;
    }

    size_t Count(void)
    {
        Clist *node = static_cast<Clist *>(this->mNext);
        size_t cnt  = 0;

        if (node)
        {
            do
            {
                node = static_cast<Clist *>(node->mNext);
                ++cnt;
            } while (node != this->mNext);
        }

        return cnt;
    }
};

} // namespace mt

#endif /* CORE_CLIST_HPP */
