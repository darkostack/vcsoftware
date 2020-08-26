#include <arduino/base.hpp>

#include <vcrtos/instance.h>
#include <vcrtos/thread.h>
#include <vcrtos/ztimer.h>
#include <vcrtos/cli.h>

static ztimer_t _timer1;
static uint32_t _timer1_counter = 0;

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

void Arduino::setup(void)
{
    Serial.begin(115200);

    Serial.println("Enable command line interface");

    vccli_uart_init(instance);
    vccli_set_user_commands(user_command_list, 2);

    _timer1.callback = _timer1_handler;
    _timer1.arg = &_timer1_counter;

    ztimer_set(ZTIMER_USEC, &_timer1, 1000000);
}

void Arduino::loop(void)
{
    thread_sleep(instance);
}
