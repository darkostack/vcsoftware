#include <stdio.h>

#include <vcrtos/instance.h>
#include <vcrtos/thread.h>
#include <vcrtos/cli.h>
#include <vcrtos/ztimer.h>

#include <vcdrivers/stdiobase.h>

#include "native_internal.h"
#include "tty_uart.h"

void cli_cmd_exit(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    tty_uart_close(VCDRIVERS_CONFIG_STDIOBASE_UART_DEV);

    real_exit(EXIT_SUCCESS);
}

static void _timer_test_handler(void *arg);
static int _timer_test_counter = 0;
static ztimer_t _timer_test = {
    .callback = _timer_test_handler,
    .arg = &_timer_test_counter
};

static void _timer_test_handler(void *arg)
{
    int *counter = (int *)arg;
    *counter += 1;
    printf("counter: %d\r\n", *counter);
    ztimer_set(ZTIMER_USEC, &_timer_test, 1000000);
}

void cli_cmd_timer_test(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    ztimer_set(ZTIMER_USEC, &_timer_test, 1000000);
}

const cli_command_t user_command_list[] = {
    { "exit", cli_cmd_exit },
    { "tt", cli_cmd_timer_test },
};

int main(void)
{
    vcstdio_init(_native_instance);

    vccli_uart_init(_native_instance);
    vccli_set_user_commands(user_command_list, 2);

    while (1)
    {
        thread_sleep(_native_instance);
    }

    return 0;
}
