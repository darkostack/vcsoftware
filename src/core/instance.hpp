#ifndef CORE_INSTANCE_HPP
#define CORE_INSTANCE_HPP

#include <stdint.h>
#include <stdbool.h>

#include <mtos/config.h>
#include <mtos/instance.h>

#include "core/thread.hpp"

typedef struct mtInstance
{
} mtInstance;

namespace mt {

class Instance : public mtInstance
{
public:
#if MTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
    static Instance &Init(void *aBuffer, size_t *aBufferSize);
#else
    static Instance &InitSingle(void);
    static Instance &Get(void);
#endif

    bool IsInitialized(void) const { return mIsInitialized; }

    template <typename Type> inline Type &Get(void);

private:
    explicit Instance(void);

    void AfterInit(void);

    ThreadScheduler mThreadScheduler;

    bool mIsInitialized;
};

template <> inline ThreadScheduler &Instance::Get(void)
{
    return mThreadScheduler;
}

} // namespace mt

#endif /* CORE_INSTANCE_HPP */
