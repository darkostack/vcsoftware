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
    ct->etimer.super.callback = _ctimer_callback;
    ct->etimer.super.arg = (void *)ct;
    timer_set(&ct->etimer.timer, interval);
    ct->cb = cb;
    ct->arg = arg;
    ztimer_set(ZTIMER_USEC, &ct->etimer.super, (uint32_t)interval);
}

void ctimer_set(struct ctimer *ct, clock_time_t interval, void (*cb)(void *), void *arg)
{
    ct->etimer.p = PROCESS_CURRENT();
    _ctimer_set(ct, interval, cb, arg);
}

void ctimer_reset(struct ctimer *ct)
{
    ztimer_remove(ZTIMER_USEC, &ct->etimer.super);
    _ctimer_set(ct, ct->etimer.timer.interval, ct->cb, ct->arg);
}

void ctimer_set_with_process(struct ctimer *ct, clock_time_t interval,
                             void (*cb)(void *), void *arg, struct process *p)
{
    ct->etimer.p = p;
    _ctimer_set(ct, interval, cb, arg);
}

void ctimer_stop(struct ctimer *ct)
{
    ztimer_remove(ZTIMER_USEC, &ct->etimer.super);
    ct->etimer.timer.start = 0;
}

int ctimer_expired(struct ctimer *ct)
{
    return etimer_expired(&ct->etimer);
}
