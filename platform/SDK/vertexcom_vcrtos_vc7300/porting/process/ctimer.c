#include "ctimer.h"

PROCESS(ctimer_process, "ctimer-process", 1024);

PROCESS_THREAD(ctimer_process, ev, data)
{
    PROCESS_BEGIN();

    while (1)
    {
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);

        if (data != NULL)
        {
            struct ctimer *ct = (struct ctimer *)data;
            ct->cb(ct->arg);
        }
    }

    PROCESS_END();
}

void ctimer_callback(void *arg)
{
    process_post(&ctimer_process, PROCESS_EVENT_TIMER, (process_data_t)arg);
}

void ctimer_init(void)
{
    process_start(&ctimer_process, NULL);
}

static void _ctimer_set(struct ctimer *ct, clock_time_t interval, void (*cb)(void *), void *arg)
{
    ct->super.callback = ctimer_callback;
    ct->super.arg = (void *)ct;

    ct->start = ztimer_now(ZTIMER_USEC);
    ct->interval = interval;
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
    ctimer_set(ct, ct->interval, ct->cb, ct->arg);
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
    ct->start = 0;
}

static int _is_strictly_before(uint32_t time_a, uint32_t time_b)
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
    return _is_strictly_before((ct->start + ct->interval), ztimer_now(ZTIMER_USEC));
}
