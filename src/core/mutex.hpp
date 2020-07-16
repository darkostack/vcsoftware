#ifndef CORE_MUTEX_HPP
#define CORE_MUTEX_HPP

#include <stddef.h>
#include <stdint.h>

#include <mtos/mutex.h>

#include "core/list.hpp"
#include "core/locator.hpp"

namespace mt {

class Mutex : public mtMutex, public InstanceLocatorInit
{
public:
    explicit Mutex(Instance &aInstance)
    {
        InstanceLocatorInit::Init(aInstance);
        mQueue.mNext = NULL;
    }

    int TryLock(void) { return SetLock(0); }

    void Lock(void) { SetLock(1); }

    void Unlock(void);

    void UnlockAndSleepingCurrentThread(void);

private:
    int SetLock(int aBlocking);
};

} // namespace mt

#endif /* CORE_MUTEX_HPP */
