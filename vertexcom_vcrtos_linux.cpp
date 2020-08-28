#include <stdio.h>

#include <vcrtos/instance.h>
#include <vcrtos/thread.h>
#include <vcrtos/cli.h>
#include <vcrtos/ztimer.h>

#include <vcdrivers/stdiobase.h>

#include "native_internal.h"
#include "tty_uart.h"

#include "process.h"
#include "etimer.h"
#include "ctimer.h"

#include "main.hpp"

PROCESS(test_process, "test-process", 12288);

void cli_cmd_exit(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    tty_uart_close(VCDRIVERS_CONFIG_STDIOBASE_UART_DEV);

    real_exit(EXIT_SUCCESS);
}

static int _timer1_counter = 0;
static void _timer1_handler(void *arg);
static ztimer_t _timer1 = {
    .callback = _timer1_handler,
    .arg = &_timer1_counter
};

static int _timer2_counter = 0;
static void _timer2_handler(void *arg);
static ztimer_t _timer2 = {
    .callback = _timer2_handler,
    .arg = &_timer2_counter
};

static void _timer1_handler(void *arg)
{
    int *counter = (int *)arg;
    *counter += 1;
    ztimer_set(ZTIMER_USEC, &_timer1, 500000);
}

static void _timer2_handler(void *arg)
{
    int *counter = (int *)arg;
    *counter += 1;
    ztimer_set(ZTIMER_USEC, &_timer2, 1000000);
}

void cli_cmd_timer_start(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    ztimer_set(ZTIMER_USEC, &_timer1, 500000);
    ztimer_set(ZTIMER_USEC, &_timer2, 1000000);
}

void cli_cmd_timer_get(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    printf("t1: %u\r\n", _timer1_counter);
    printf("t2: %u\r\n", _timer2_counter);
}

void cli_cmd_process_timer(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    process_start(&test_process, NULL);
}

const cli_command_t user_command_list[] = {
    { "exit", cli_cmd_exit },
    { "ts", cli_cmd_timer_start },
    { "tg", cli_cmd_timer_get },
    { "pt", cli_cmd_process_timer },
};

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
        printf("e: %u\r\n", etimer_counter++);
        printf("c: %u\r\n", _ctimer_counter);
    }

    PROCESS_END();
}

void Main::setup(void)
{
    vcstdio_init(instance);

    vccli_uart_init(instance);
    vccli_set_user_commands(user_command_list, 4);

    process_init(instance);
}

void Main::loop(void)
{
    thread_sleep(instance);
}
