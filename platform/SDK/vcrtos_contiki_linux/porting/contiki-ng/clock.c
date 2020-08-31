#include "contiki.h"

void clock_init(void)
{
}

clock_time_t clock_time(void)
{
    return ztimer_now(ZTIMER_USEC);
}

unsigned long clock_seconds(void)
{
    return (unsigned long)(clock_time() / CLOCK_CONF_SECOND);
}

void clock_wait(clock_time_t i)
{
    ztimer_sleep(ZTIMER_USEC, i);
}

void clock_delay_usec(uint16_t dt)
{
    (void) dt;
    vcassert(0); /* use clock_wait */
}

void clock_delay(unsigned int i)
{
    (void) i;
    vcassert(0); /* use clock_wait */
}
