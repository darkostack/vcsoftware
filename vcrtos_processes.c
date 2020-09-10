#include <stdio.h>
#include <inttypes.h>

#include <vcrtos/instance.h>
#include <vcrtos/thread.h>
#include <vcrtos/ztimer.h>
#include <vcrtos/cli.h>

#include "contiki.h"

PROCESS(app_process, "app-process", VCRTOS_CONFIG_MAIN_THREAD_STACK_SIZE);

AUTOSTART_PROCESSES(&app_process);

//static struct etimer timer_etimer;
//static uint32_t etimer_counter = 0;

PROCESS_THREAD(app_process, ev, data)
{
    PROCESS_BEGIN();

    printf("app-process start\r\n");

    //etimer_set(&timer_etimer, CLOCK_SECOND);

    while (1)
    {
        PROCESS_YIELD();

        (void) ev;

        //PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
        //etimer_set(&timer_etimer, CLOCK_SECOND);
        //printf("e: %" PRIu32 "\r\n", etimer_counter++);
    }

    PROCESS_END();
}
