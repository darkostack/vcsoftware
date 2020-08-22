#include <vcrtos/cpu.h>
#include <vcrtos/ztimer/periph_timer.h>

#include <vcdrivers/periph/tim.h>

static void _ztimer_periph_timer_set(ztimer_clock_t *clock, uint32_t val)
{
    ztimer_periph_timer_t *ztimer_periph = (ztimer_periph_timer_t *)clock;

    uint16_t min = ztimer_periph->min;

    if (val < min)
    {
        val = min;
    }

    unsigned state = cpu_irq_disable();

    vctim_set(TIM_DEV(ztimer_periph->dev), 0, val);

    cpu_irq_restore(state);
}

static uint32_t _ztimer_periph_timer_now(ztimer_clock_t *clock)
{
    ztimer_periph_timer_t *ztimer_periph = (ztimer_periph_timer_t *)clock;

    return vctim_read(TIM_DEV(ztimer_periph->dev));
}

static void _ztimer_periph_timer_cancel(ztimer_clock_t *clock)
{
    ztimer_periph_timer_t *ztimer_periph = (ztimer_periph_timer_t *)clock;

    vctim_clear(TIM_DEV(ztimer_periph->dev), 0);
}

static void _ztimer_periph_timer_callback(void *arg, int channel)
{
    (void)channel;
    ztimer_handler((ztimer_clock_t *)arg);
}

static const ztimer_ops_t _ztimer_periph_timer_ops = {
    .set = _ztimer_periph_timer_set,
    .now = _ztimer_periph_timer_now,
    .cancel = _ztimer_periph_timer_cancel,
};

void ztimer_periph_timer_init(ztimer_periph_timer_t *clock,
                              unsigned int dev,
                              unsigned long freq,
                              uint32_t max_val)
{
    clock->dev = dev;
    clock->super.ops = &_ztimer_periph_timer_ops;
    clock->super.max_value = max_val;
    vctim_init(TIM_DEV(dev), freq, _ztimer_periph_timer_callback, clock);
    ztimer_init_extend(&clock->super);
}
