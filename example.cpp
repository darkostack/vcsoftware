#include <arduino/base.hpp>

#include <vcrtos/instance.h>
#include <vcrtos/thread.h>
#include <vcrtos/xtimer.h>
#include <vcrtos/cli.h>

#include "process.h"

static instance_t *_instance;
static xtimer_t timer_test;
static uint32_t counter = 0;

PROCESS(test, "test", 1024);

void xtimer_test_handler(void *arg)
{
    uint32_t *cnt = static_cast<uint32_t *>(arg);

    *cnt += 1;

    printf("%lu\r\n", static_cast<uint32_t>(*cnt));

    xtimer_set(&timer_test, 1000000);
}

void setup(void)
{
    Serial.begin(115200);

    Serial.println("Enable command line interface");

    _instance = instance_get();

    vccli_uart_init(_instance);

    xtimer_init(_instance, &timer_test, xtimer_test_handler, static_cast<void *>(&counter));
    xtimer_set(&timer_test, 1000000);

    process_init(_instance);
    process_start(&test, NULL);
}

void loop(void)
{
    thread_yield(_instance);
}

PROCESS_THREAD(test, ev, data)
{
    PROCESS_BEGIN();

    while (1)
    {
        PROCESS_WAIT_EVENT();
        PROCESS_YIELD();
    }

    PROCESS_END();
}
