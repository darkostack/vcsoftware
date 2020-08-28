#include <arduino/base.hpp>

#include <vcrtos/instance.h>
#include <vcrtos/thread.h>
#include <vcrtos/cli.h>

#include "main.hpp"

extern "C" void vcrtos_cmd_ps(int argc, char **argv);

const cli_command_t user_command_list[] = {
    { "ps", vcrtos_cmd_ps },
};

void Main::setup(void)
{
    Serial.begin(115200);
    Serial.println("Enable command line interface");

    vccli_uart_init(instance);
    vccli_set_user_commands(user_command_list, 1);
}

void Main::loop(void)
{
    thread_sleep(instance);
}
