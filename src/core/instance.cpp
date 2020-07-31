#include "core/instance.hpp"
#include "core/code_utils.hpp"
#include "core/new.hpp"

namespace vc {

#if !VCOS_CONFIG_MULTIPLE_INSTANCE_ENABLE

DEFINE_ALIGNED_VAR(instance_raw, sizeof(Instance), uint64_t);

#endif /* #if !VCOS_CONFIG_MULTIPLE_INSTANCE_ENABLE */

Instance::Instance(void)
    : initialized(false)
    , thread_scheduler()
    , thread_flags(this->thread_scheduler)
{
}

#if VCOS_CONFIG_MULTIPLE_INSTANCE_ENABLE

Instance &Instance::init(void *buffer, size_t *size)
{
    Instance *instance = NULL;

    VERIFY_OR_EXIT(size != NULL);

    VERIFY_OR_EXIT(sizeof(Instance) <= *size, *size = sizeof(Instance));

    VERIFY_OR_EXIT(buffer != NULL);

    instance = new (buffer) Instance();

    instance->after_init();

exit:
    return *instance;
}

#else /* #if VCOS_CONFIG_MULTIPLE_INSTANCE_ENABLE */

Instance &Instance::init_single(void)
{
    Instance *instance = &get();

    VERIFY_OR_EXIT(instance->is_initialized == false);

    instance = new (&instance_raw) Instance();

    instance->after_init();

exit:
    return *instance;
}

Instance &Instance::get(void)
{
    void *instance = &instance_raw;
    return *static_cast<Instance *>(instance);
}

#endif /* #if VCOS_CONFIG_MULTIPLE_INSTANCE_ENABLE */

void Instance::after_init(void)
{
    initialized = true;
}

} // namespace vc
