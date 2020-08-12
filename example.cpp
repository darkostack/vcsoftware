#include <arduino/base.hpp>

#include <vcrtos/instance.h>
#include <vcrtos/thread.h>
#include <vcrtos/cli.h>

static instance_t *_instance;

void setup(void)
{
    Serial.begin(115200);

    Serial.println("Enable command line interface");

    _instance = instance_get();

    vccli_uart_init(_instance);
}


void loop(void)
{
    thread_yield(_instance);
}
