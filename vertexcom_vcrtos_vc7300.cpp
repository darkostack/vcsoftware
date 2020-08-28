#include <arduino/base.hpp>

#include <vcrtos/instance.h>
#include <vcrtos/thread.h>
#include <vcrtos/ztimer.h>
#include <vcrtos/cli.h>

#include "process.h"
#include "etimer.h"
#include "ctimer.h"

#include "main.hpp"

static ztimer_t _timer1;
static uint32_t _timer1_counter = 0;

PROCESS(test_process, "test-process", 1024);

extern "C" void vcrtos_cmd_ps(int argc, char **argv);

void _get_timer_counter(int argc, char **argv)
{
    (void) argc;
    (void) argv;
    printf("counter: %lu\r\n", _timer1_counter);
}

const cli_command_t user_command_list[] = {
    { "ps", vcrtos_cmd_ps },
    { "tc", _get_timer_counter },
};

static void _timer1_handler(void *arg)
{
    uint32_t *counter = static_cast<uint32_t *>(arg);
    *counter += 1;
    ztimer_set(ZTIMER_USEC, &_timer1, 1000000);
}

static uint32_t _ctimer_counter = 0;

static void _ctimer_handler(void *arg)
{
    struct ctimer *ct = (struct ctimer *)arg;
    _ctimer_counter += 1;
    ctimer_reset(ct);
}

PROCESS_THREAD(test_process, ev, data)
{
    static struct etimer timer_etimer;
    static struct ctimer timer_ctimer;

    PROCESS_BEGIN();

    uint32_t etimer_counter = 0;

    ctimer_set(&timer_ctimer, 500000, _ctimer_handler, &timer_ctimer);

    while (1)
    {
        etimer_set(&timer_etimer, 1000000);
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
        printf("e: %lu\r\n", etimer_counter++);
        printf("c: %lu\r\n", _ctimer_counter);
    }

    PROCESS_END();
}

void Main::setup(void)
{
    Serial.begin(115200);

    Serial.println("Enable command line interface");

    vccli_uart_init(instance);
    vccli_set_user_commands(user_command_list, 2);

    process_init(instance);
    process_start(&test_process, NULL);

    _timer1.callback = _timer1_handler;
    _timer1.arg = &_timer1_counter;

    ztimer_set(ZTIMER_USEC, &_timer1, 1000000);
}

void Main::loop(void)
{
    thread_sleep(instance);
}
