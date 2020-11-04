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

#include <vcdrivers/cpu.h>

extern uint32_t SystemCoreClock;

static volatile uint32_t _systick_count = 0;

void clock_init(void)
{
    SysTick_Config(SystemCoreClock / 1000); /* 1 millisecond systick */

    NVIC_EnableIRQ(SysTick_IRQn);
}

clock_time_t clock_time(void)
{
    return _systick_count;
}

uint32_t clock_time_usec(void)
{
    return (_systick_count * 1000) + 1000 - (SysTick->VAL / (SystemCoreClock / 1000000));
}

unsigned long clock_seconds(void)
{
    return (unsigned long)(clock_time() / CLOCK_CONF_SECOND);
}

void clock_wait(clock_time_t i)
{
    while (clock_time() < i);
}

void clock_delay_usec(uint16_t dt)
{
    uint32_t timeout = clock_time_usec() + dt;
    while (clock_time_usec() < timeout);
}

void clock_delay(unsigned int i)
{
    clock_wait(i);
}

void isr_systick(void)
{
    _systick_count++;

    if (etimer_pending())
    {
        etimer_request_poll();
    }

    cpu_end_of_isr();
}
