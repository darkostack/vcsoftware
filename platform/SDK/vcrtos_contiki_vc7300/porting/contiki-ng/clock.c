/*
 * Copyright (c) 2020, Vertexcom Technologies, Inc.
 * All rights reserved.
 *
 * NOTICE: All information contained herein is, and remains
 * the property of Vertexcom Technologies, Inc. and its suppliers,
 * if any. The intellectual and technical concepts contained
 * herein are proprietary to Vertexcom Technologies, Inc.
 * and may be covered by U.S. and Foreign Patents, patents in process,
 * and protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Vertexcom Technologies, Inc.
 *
 * Authors: Darko Pancev <darko.pancev@vertexcom.com>
 */

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
