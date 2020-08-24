#include <arduino/base.hpp>

#include <vcrtos/instance.h>
#include <vcrtos/thread.h>
#include <vcrtos/ztimer.h>
#include <vcrtos/cli.h>

extern "C" void vcrtos_cmd_ps(int argc, char **argv);

const cli_command_t user_command_list[] = {
    { "ps", vcrtos_cmd_ps },
};

static ztimer_t _timer1;
static uint32_t _timer1_counter = 0;

static void _timer1_handler(void *arg)
{
    uint32_t *counter = static_cast<uint32_t *>(arg);
    *counter += 1;
    printf("counter: %lu\r\n", static_cast<uint32_t>(*counter));
    ztimer_set(ZTIMER_USEC, &_timer1, 1000000);
}

void Arduino::setup(void)
{
    Serial.begin(115200);

    Serial.println("Enable command line interface");

    vccli_uart_init(instance);
    vccli_set_user_commands(user_command_list, 1);

    _timer1.callback = _timer1_handler;
    _timer1.arg = &_timer1_counter;

    ztimer_set(ZTIMER_USEC, &_timer1, 1000000);
}

void Arduino::loop(void)
{
    thread_sleep(instance);
}
