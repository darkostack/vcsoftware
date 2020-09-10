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

#include <stdio.h>
#include <inttypes.h>

#include <vcrtos/instance.h>
#include <vcrtos/thread.h>
#include <vcrtos/ztimer.h>
#include <vcrtos/cli.h>

#include "contiki.h"

PROCESS(app_process, "app-process", VCRTOS_CONFIG_MAIN_THREAD_STACK_SIZE);

AUTOSTART_PROCESSES(&app_process);

static struct etimer timer_etimer;
static uint32_t etimer_counter = 0;

PROCESS_THREAD(app_process, ev, data)
{
    PROCESS_BEGIN();

    printf("app-process start\r\n");

    etimer_set(&timer_etimer, CLOCK_SECOND);

    while (1)
    {
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
        etimer_set(&timer_etimer, CLOCK_SECOND);
        printf("e: %" PRIu32 "\r\n", etimer_counter++);
    }

    PROCESS_END();
}
