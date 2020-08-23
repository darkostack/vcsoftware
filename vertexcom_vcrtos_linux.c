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

PROCESS(test_process, "test-process", 4096);

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
    printf("t1: %d\r\n", *counter);
    ztimer_set(ZTIMER_USEC, &_timer1, 500000);
}

static void _timer2_handler(void *arg)
{
    int *counter = (int *)arg;
    *counter += 1;
    printf("--t2: %d\r\n", *counter);
    ztimer_set(ZTIMER_USEC, &_timer2, 1000000);
}

void cli_cmd_timer_test(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    ztimer_set(ZTIMER_USEC, &_timer1, 500000);
    ztimer_set(ZTIMER_USEC, &_timer2, 1000000);
}

void cli_cmd_etimer_test(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    process_start(&test_process, NULL);
}

const cli_command_t user_command_list[] = {
    { "exit", cli_cmd_exit },
    { "timer", cli_cmd_timer_test },
    { "etimer", cli_cmd_etimer_test },
};

PROCESS_THREAD(test_process, ev, data)
{
    static struct etimer timer_etimer;

    PROCESS_BEGIN();

    uint32_t etimer_counter = 0;

    while (1)
    {
        etimer_set(&timer_etimer, 1000000);
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
        printf("e: %u\r\n", etimer_counter++);
    }

    PROCESS_END();
}

int main(void)
{
    vcstdio_init(_native_instance);

    vccli_uart_init(_native_instance);
    vccli_set_user_commands(user_command_list, 3);

    process_init(_native_instance);
    ctimer_init();

    while (1)
    {
        thread_sleep(_native_instance);
    }

    return 0;
}
