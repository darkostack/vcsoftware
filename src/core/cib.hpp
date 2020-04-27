#ifndef CORE_CIB_HPP
#define CORE_CIB_HPP

#include <assert.h>

#include <mtos/cib.h>

namespace mt {

class Cib : public mtCib
{
public:
    explicit Cib(unsigned int aSize) { Init(aSize); }

    void Init(unsigned int aSize)
    {
        assert(!(aSize & (aSize - 1)));
        mReadCount = 0;
        mWriteCount = 0;
        mMask = aSize - 1;
    }

    unsigned int Avail(void) { return mWriteCount - mReadCount; }

    unsigned int GetReadCount(void) { return mReadCount; }

    unsigned int GetWriteCount(void) { return mWriteCount; }

    unsigned int GetMask(void) { return mMask; }

    int GetUnsafe(void) { return static_cast<int>(mReadCount++ & mMask); }

    int PutUnsafe(void) { return static_cast<int>(mWriteCount++ & mMask); }

    int Full(void) { return Avail() > static_cast<int>(mMask); }

    int Get(void)
    {
        if (Avail())
        {
            return static_cast<int>(mReadCount++ & mMask);
        }

        return -1;
    }

    int Peek(void)
    {
        if (Avail())
        {
            return static_cast<int>(mReadCount & mMask);
        }

        return -1;
    }

    int Put(void)
    {
        int avail = Avail();

        if (avail <= static_cast<int>(mMask))
        {
            return static_cast<int>(mWriteCount++ & mMask);
        }

        return -1;
    }
};

} // namespace mt

#endif /* CORE_COMMON_CIB_HPP */
