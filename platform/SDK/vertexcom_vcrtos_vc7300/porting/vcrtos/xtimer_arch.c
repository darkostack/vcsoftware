#include <vcdrivers/config.h>
#include <vcdrivers/periph/tim.h>

int lltimer_init(unsigned int dev, unsigned long freq, void(*callback)(void *, int), void *arg)
{
    return vctim_init((vctim_t)dev, freq, (vctim_callback_func_t)callback, arg);
}

int lltimer_set_absolute(unsigned int dev, unsigned channel, unsigned int value)
{
    return vctim_set_absolute((vctim_t)dev, channel, value);
}

uint32_t lltimer_read(unsigned int dev)
{
    return vctim_read((vctim_t)dev);
}
