#include <stdio.h>

#include <vcrtos/instance.h>
#include <vcrtos/thread.h>
#include <vcrtos/cli.h>
#include <vcrtos/ztimer.h>

#include <vcdrivers/stdiobase.h>

#include "native_internal.h"
#include "tty_uart.h"

#include "main.hpp"

#include "process.h"

PROCESS_NAME(test_process);

void cli_cmd_exit(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    tty_uart_close(VCDRIVERS_CONFIG_STDIOBASE_UART_DEV);

    real_exit(EXIT_SUCCESS);
}

const cli_command_t user_command_list[] = {
    { "exit", cli_cmd_exit },
};

void Main::setup(void)
{
    vcstdio_init(instance);

    vccli_uart_init(instance);
    vccli_set_user_commands(user_command_list, 1);

    process_init(instance);
    process_start(&test_process, NULL);
}

void Main::loop(void)
{
    thread_sleep(instance);
}
