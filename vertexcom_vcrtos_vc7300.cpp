#include <arduino/base.hpp>

#include <vcrtos/instance.h>
#include <vcrtos/thread.h>
#include <vcrtos/ztimer.h>
#include <vcrtos/cli.h>

//#include "process.h"
//#include "etimer.h"
//#include "ctimer.h"

extern "C" void vcrtos_cmd_ps(int argc, char **argv);

const cli_command_t user_command_list[] = {
    { "ps", vcrtos_cmd_ps },
};

static void _timer_test_handler(void *arg);
static uint32_t _timer_test_counter = 0;
static ztimer_t _timer_test;

//static uint32_t ctimer_counter = 0;

//PROCESS(test_process, "test-process", 1024);

static void _timer_test_handler(void *arg)
{
    //struct process *p = static_cast<struct process *>(arg);
    //process_post(p, PROCESS_EVENT_TIMER, NULL);

    uint32_t *counter = static_cast<uint32_t *>(arg);
    *counter += 1;
    printf("fired: %lu\n", static_cast<uint32_t>(*counter));
    ztimer_set(ZTIMER_USEC, &_timer_test, 1000000);
}

void Arduino::setup(void)
{
    Serial.begin(115200);

    Serial.println("Enable command line interface");

    vccli_uart_init(instance);
    vccli_set_user_commands(user_command_list, 1);

    //process_init(instance);
    //ctimer_init();
    //process_start(&test_process, NULL);

    _timer_test.callback = _timer_test_handler;
    _timer_test.arg = &_timer_test_counter;

    ztimer_set(ZTIMER_USEC, &_timer_test, 1000000);
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
