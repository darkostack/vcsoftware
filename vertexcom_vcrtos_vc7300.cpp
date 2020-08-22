#include <arduino/base.hpp>

#include <vcrtos/instance.h>
#include <vcrtos/thread.h>
//#include <vcrtos/xtimer.h>
#include <vcrtos/cli.h>

//#include "process.h"
//#include "etimer.h"
//#include "ctimer.h"

extern "C" void vcrtos_cmd_ps(int argc, char **argv);

const cli_command_t user_command_list[] = {
    { "ps", vcrtos_cmd_ps },
};

//static xtimer_t timer_test;

//static uint32_t ctimer_counter = 0;

//PROCESS(test_process, "test-process", 1024);

#if 0
void xtimer_test_handler(void *arg)
{
    struct process *p = static_cast<struct process *>(arg);

    process_post(p, PROCESS_EVENT_TIMER, NULL);

    xtimer_set(&timer_test, 1000000);
}
#endif

void Arduino::setup(void)
{
    Serial.begin(115200);

    Serial.println("Enable command line interface");

    vccli_uart_init(instance);
    vccli_set_user_commands(user_command_list, 1);

    //process_init(instance);
    //ctimer_init();
    //process_start(&test_process, NULL);

    //xtimer_init(_instance, &timer_test, xtimer_test_handler, static_cast<void *>(&test_process));
    //xtimer_set(&timer_test, 1000000);
}

void Arduino::loop(void)
{
    //thread_yield(instance);
    thread_sleep(instance);
}

#if 0
void ctimer_timeout(void *arg)
{
    struct ctimer *ct = (struct ctimer *)arg;

    printf("--c: %lu\r\n", ctimer_counter++);

    ctimer_reset(ct);
}

PROCESS_THREAD(test_process, ev, data)
{
    static struct etimer timer_etimer;
    static struct ctimer timer_ctimer;

    PROCESS_BEGIN();

    uint32_t etimer_counter = 0;

    ctimer_set(&timer_ctimer, 2 * 1000000, ctimer_timeout, &timer_ctimer);

    while (1)
    {
        etimer_set(&timer_etimer, 1000000);
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
        printf("e: %lu\r\n", etimer_counter++);
    }

    PROCESS_END();
}
#endif
