#ifndef CORE_LOCATOR_HPP
#define CORE_LOCATOR_HPP

#include "core/config/mtos-core-config.h"

#include <stdint.h>

namespace mt {

class Instance;

#if !MTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
extern uint64_t gInstanceRaw[];
#endif

class InstanceLocator
{
    friend class InstanceLocatorInit;

public:

#if MTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
    Instance &GetInstance(void) const { return *mInstance; }
#else
    Instance &GetInstance(void) const { return *reinterpret_cast<Instance *>(&gInstanceRaw); }
#endif

    template <typename Type> inline Type &Get(void) const; // Implemented in `locator-getters.hpp`.

protected:

    explicit InstanceLocator(Instance &aInstance)
#if MTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
        : mInstance(&aInstance)
#endif
    {
        (void) aInstance;
    }

private:
    InstanceLocator(void) {}

#if MTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
    Instance *mInstance;
#endif
};

class InstanceLocatorInit : public InstanceLocator
{
protected:
    InstanceLocatorInit(void)
        : InstanceLocator()
    {
    }

    void Init(Instance &aInstance)
    {
#if MTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
        mInstance = &aInstance;
#endif
        (void) aInstance;
    }
};

} // namespace mt

#endif /* CORE_LOCATOR_HPP */
