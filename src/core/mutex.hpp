#ifndef CORE_MUTEX_HPP
#define CORE_MUTEX_HPP

#include <stddef.h>
#include <stdint.h>

#include <mtos/config.h>
#include <mtos/mutex.h>

#include "core/list.hpp"

namespace mt {

class Instance;

#if !MTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
extern uint64_t gInstanceRaw[];
#endif

class Mutex : public mtMutex
{
public:
    Mutex(void) {}

    explicit Mutex(Instance &aInstance)
    {
        Init(aInstance);
    }

    void Init(Instance &aInstance)
    {
#if MTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
        mInstance = static_cast<void *>(&aInstance);
#else
        (void)aInstance;
#endif
        mQueue.mNext = NULL;
    }

    int TryLock(void) { return SetLock(0); }

    void Lock(void) { SetLock(1); }

    void Unlock(void);

    void UnlockAndSleepingCurrentThread(void);

    template <typename Type> inline Type &Get(void) const; 

private:
    int SetLock(int aBlocking);

#if MTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
    Instance &GetInstance(void) const { return *static_cast<Instance *>(mInstance); }
#else
    Instance &GetInstance(void) const { return *reinterpret_cast<Instance *>(&gInstanceRaw); }
#endif
};

} // namespace mt

#endif /* CORE_MUTEX_HPP */
