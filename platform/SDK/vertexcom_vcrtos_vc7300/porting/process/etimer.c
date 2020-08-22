#include "etimer.h"

void etimer_callback(void *arg)
{
    struct etimer *et = (struct etimer *)arg;

    if (et->p != PROCESS_NONE)
    {
        process_post(et->p, PROCESS_EVENT_TIMER, NULL);
        et->p = PROCESS_NONE;
    }
}

void etimer_set(struct etimer *et, clock_time_t interval)
{
    et->super.callback = etimer_callback;
    et->super.arg = (void *)et;

    et->p = PROCESS_CURRENT();
    et->start = ztimer_now(ZTIMER_USEC);
    et->interval = interval;

    ztimer_set(ZTIMER_USEC, &et->super, (uint32_t)interval);
}

void etimer_reset(struct etimer *et)
{
    if (et->p != PROCESS_NONE)
    {
        ztimer_remove(ZTIMER_USEC, &et->super);
    }
    etimer_set(et, et->interval);
}

void etimer_reset_with_new_interval(struct etimer *et, clock_time_t interval)
{
    if (et->p != PROCESS_NONE)
    {
        ztimer_remove(ZTIMER_USEC, &et->super);
    }

    etimer_set(et, interval);
}

void etimer_restart(struct etimer *et)
{
    etimer_reset(et);
}

clock_time_t etimer_expiration_time(struct etimer *et)
{
    return et->start + et->interval;
}

clock_time_t etimer_start_time(struct etimer *et)
{
    return et->start;
}

int etimer_expired(struct etimer *et)
{
    return et->p == PROCESS_NONE;
}

void etimer_stop(struct etimer *et)
{
    et->p = PROCESS_NONE;
}
