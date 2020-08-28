#include "core/code_utils.h"
#include "core/instance.hpp"
#include "core/new.hpp"

#include "main.hpp"

static DEFINE_ALIGNED_VAR(main_raw, sizeof(Main), uint64_t);

int main(void)
{
    void *instance = static_cast<void *>(instance_get());

    Main *base = new (&main_raw) Main(instance);

    base->setup();

    while (1)
    {
        base->loop();
    }
}
