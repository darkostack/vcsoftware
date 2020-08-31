#include "ctimer.h"

static void _ctimer_callback(void *arg)
{
    struct ctimer *ct = (struct ctimer *)arg;
    ct->cb(ct->arg);
}

void ctimer_init(void)
{
}

static void _ctimer_set(struct ctimer *ct, clock_time_t interval, void (*cb)(void *), void *arg)
{
    ct->super.callback = _ctimer_callback;
    ct->super.arg = (void *)ct;
    timer_set(&ct->timer, interval);
    ct->cb = cb;
    ct->arg = arg;
    ztimer_set(ZTIMER_USEC, &ct->super, (uint32_t)interval);
}

void ctimer_set(struct ctimer *ct, clock_time_t interval, void (*cb)(void *), void *arg)
{
    ct->p = PROCESS_CURRENT();
    _ctimer_set(ct, interval, cb, arg);
}

void ctimer_reset(struct ctimer *ct)
{
    ztimer_remove(ZTIMER_USEC, &ct->super);
    _ctimer_set(ct, ct->timer.interval, ct->cb, ct->arg);
}

void ctimer_set_with_process(struct ctimer *ct, clock_time_t interval,
                             void (*cb)(void *), void *arg, struct process *p)
{
    ct->p = p;
    _ctimer_set(ct, interval, cb, arg);
}

void ctimer_stop(struct ctimer *ct)
{
    ztimer_remove(ZTIMER_USEC, &ct->super);
    ct->timer.start = 0;
}

static int _ctimer_strictly_before(uint32_t time_a, uint32_t time_b)
{
    uint32_t diff = time_a - time_b;

    // three cases:
    // 1. time a is before time b  => difference is negative
    //    (last bit of diff is set) return TRUE.
    // 2. time a is same as time b => difference is zero
    //    (last bit of diff is clear) return FALSE.
    // 3. time a is after time b   => difference is positive
    //    (last bif of diff is clear) return FALSE.

    return ((diff & (1UL << 31)) != 0);
}

int ctimer_expired(struct ctimer *ct)
{
    return _ctimer_strictly_before((ct->timer.start + ct->timer.interval), ztimer_now(ZTIMER_USEC));
}
