#include <stdint.h>
#include <stdio.h>

#include <vcrtos/instance.h>
#include <vcrtos/thread.h>
#include <vcrtos/cli.h>

int main(void)
{
    instance_t *instance = instance_get();

    vccli_uart_init(instance);

    while(1)
    {
        thread_yield(instance);
    }

    return 0;
}
