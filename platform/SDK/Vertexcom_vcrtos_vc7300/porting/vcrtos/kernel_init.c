#include <stdlib.h>
#include <stdio.h>

#include <vcdrivers/stdiobase.h>

#include <vcrtos/config.h>
#include <vcrtos/assert.h>
#include <vcrtos/cpu.h>
#include <vcrtos/instance.h>
#include <vcrtos/thread.h>

extern int main(void);

void *thread_main_handler(void *arg)
{
    (void) arg;

    main();

    /* should not reach here */

    vcassert(0);

    return NULL;
}

void *thread_idle_handler(void *arg)
{
    printf("idle thread handler!\r\n");

    (void) arg;

    while (1)
    {
        cpu_sleep(0);
    }

    return NULL;
}

char _main_stack[VCRTOS_CONFIG_MAIN_THREAD_STACK_SIZE];
char _idle_stack[VCRTOS_CONFIG_IDLE_THREAD_STACK_SIZE];

void kernel_init(void)
{
    (void) cpu_irq_disable();

    instance_t *instance = instance_init_single();

    vcassert(instance_is_initialized(instance));

    vcstdio_init(instance);

    printf("\r\n\r\nvcrtos kernel started\r\n\r\n");

    (void) thread_create((void *)instance, _main_stack, sizeof(_main_stack),
                         KERNEL_THREAD_PRIORITY_MAIN,
                         THREAD_FLAGS_CREATE_WOUT_YIELD | THREAD_FLAGS_CREATE_STACKMARKER,
                         thread_main_handler, (void *)instance, "main");

    (void) thread_create((void *)instance, _idle_stack, sizeof(_idle_stack),
                         KERNEL_THREAD_PRIORITY_IDLE,
                         THREAD_FLAGS_CREATE_WOUT_YIELD | THREAD_FLAGS_CREATE_STACKMARKER,
                         thread_idle_handler, (void *)instance, "idle");

    cpu_switch_context_exit();
}
