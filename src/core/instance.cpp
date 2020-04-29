#include "core/instance.hpp"
#include "core/code_utils.hpp"
#include "core/new.hpp"

namespace mt {

#if !MTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE

mtDEFINE_ALIGNED_VAR(gInstanceRaw, sizeof(Instance), uint64_t);

#endif /* #if !MTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE */

Instance::Instance(void)
    : mIsInitialized(false)
    , mThreadScheduler()
{
}

#if MTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE

Instance &Instance::Init(void *aBuffer, size_t *aBufferSize)
{
    Instance *instance = NULL;

    VerifyOrExit(aBufferSize != NULL);

    VerifyOrExit(sizeof(Instance) <= *aBufferSize, *aBufferSize = sizeof(Instance));

    VerifyOrExit(aBuffer != NULL);

    instance = new (aBuffer) Instance();

    instance->AfterInit();

exit:
    return *instance;
}

#else /* #if MTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE */

Instance &Instance::InitSingle(void)
{
    Instance *instance = &Get();

    VerifyOrExit(instance->mIsInitialized == false);

    instance = new (&gInstanceRaw) Instance();

    instance->AfterInit();
exit:
    return *instance;
}

Instance &Instance::Get(void)
{
    void *instance = &gInstanceRaw;

    return *static_cast<Instance *>(instance);
}

#endif /* #if MTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE */

void Instance::AfterInit(void)
{
    mIsInitialized = true;
}

} // namespace mt
