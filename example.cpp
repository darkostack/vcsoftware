#include <arduino/base.hpp>

#include <vcrtos/instance.h>
#include <vcrtos/thread.h>
#include <vcrtos/xtimer.h>
#include <vcrtos/cli.h>

#include "process.h"

static instance_t *_instance;
static xtimer_t timer_test;

PROCESS(test_process, "test_process", 1024);

void xtimer_test_handler(void *arg)
{
    struct process *p = static_cast<struct process *>(arg);

    process_post(p, PROCESS_EVENT_TIMER, NULL);

    xtimer_set(&timer_test, 1000000);
}

void setup(void)
{
    Serial.begin(115200);

    Serial.println("Enable command line interface");

    _instance = instance_get();

    vccli_uart_init(_instance);

    process_init(_instance);
    process_start(&test_process, NULL);

    xtimer_init(_instance, &timer_test, xtimer_test_handler, static_cast<void *>(&test_process));
    xtimer_set(&timer_test, 1000000);
}

void loop(void)
{
    thread_yield(_instance);
}

PROCESS_THREAD(test_process, ev, data)
{
    PROCESS_BEGIN();

    uint32_t timer_counter = 0;

    while (1)
    {
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
        printf("event timer: %lu\r\n", timer_counter++);
    }

    PROCESS_END();
}
