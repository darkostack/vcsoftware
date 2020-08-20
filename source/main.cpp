#include "core/code_utils.h"
#include "core/instance.hpp"
#include "core/new.hpp"

#include <arduino/base.hpp>

static DEFINE_ALIGNED_VAR(arduino_raw, sizeof(Arduino), uint64_t);

Arduino *arduino_base;

int main(void)
{
    void *instance = static_cast<void *>(instance_get());

    arduino_base = new (&arduino_raw) Arduino(instance);

    arduino_base->setup();

    while (1)
    {
        arduino_base->loop();
    }
}
