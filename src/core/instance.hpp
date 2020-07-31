#ifndef CORE_INSTANCE_HPP
#define CORE_INSTANCE_HPP

#include <stdint.h>
#include <stdbool.h>

#include <vcos/config.h>
#include <vcos/instance.h>

#include "core/thread.hpp"
#include "core/event.hpp"

typedef struct instance
{
} instance_t;

namespace vc {

class Instance : public instance_t
{
public:
#if VCOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
    static Instance &init(void *buffer, size_t *size);
#else
    static Instance &init_single(void);
    static Instance &get(void);
#endif

    bool is_initialized(void) const { return initialized; }

    template <typename Type> inline Type &get(void);

private:
    explicit Instance(void);

    void after_init(void);

    ThreadScheduler thread_scheduler;

    ThreadFlags thread_flags;

    EventQueue event_queue;

    bool initialized;
};

template <> inline ThreadScheduler &Instance::get(void)
{
    return thread_scheduler;
}

template <> inline ThreadFlags &Instance::get(void)
{
    return thread_flags;
}

template <> inline EventQueue &Instance::get(void)
{
    return event_queue;
}

} // namespace vc

#endif /* CORE_INSTANCE_HPP */
