#include <vcrtos/config.h>
#include <vcrtos/assert.h>
#include <vcrtos/cpu.h>
#include <vcrtos/instance.h>
#include <vcrtos/thread.h>

#if VCRTOS_CONFIG_THREAD_EVENT_ENABLE
#include <vcrtos/event.h>
#endif

#include "native_internal.h"

extern int main(void);

void *thread_main_handler(void *arg)
{
    (void) arg;

    main();

    /* should not reach here! */

    vcassert(0);

    return NULL;
}

void native_cpu_sleep(void)
{
    _native_in_syscall++;
    real_pause();
    _native_in_syscall--;

    if (_native_sigpend > 0)
    {
        _native_in_syscall++;
        _native_syscall_leave();
    }
}

void *thread_idle_handler(void *arg)
{
    (void) arg;

    printf("idle handler!!!\n");

    while (1)
    {
        native_cpu_sleep();
    }

    return NULL;
}

char _main_stack[VCRTOS_CONFIG_MAIN_THREAD_STACK_SIZE];
char _idle_stack[VCRTOS_CONFIG_IDLE_THREAD_STACK_SIZE];

void _kernel_init(void *instance)
{
    (void) cpu_irq_disable();

    vcassert(instance_is_initialized(instance));

    printf("\n\nvcrtos-%s kernel started\n\n", VCRTOS_VERSION);

    kernel_pid_t main_pid = thread_create((void *)instance, _main_stack, sizeof(_main_stack),
                         KERNEL_THREAD_PRIORITY_MAIN,
                         THREAD_FLAGS_CREATE_WOUT_YIELD | THREAD_FLAGS_CREATE_STACKMARKER,
                         thread_main_handler, (void *)instance, "main");

    printf("main pid: %d\n", (int)main_pid);

    kernel_pid_t idle_pid = thread_create((void *)instance, _idle_stack, sizeof(_idle_stack),
                         KERNEL_THREAD_PRIORITY_IDLE,
                         THREAD_FLAGS_CREATE_WOUT_YIELD | THREAD_FLAGS_CREATE_STACKMARKER,
                         thread_idle_handler, (void *)instance, "idle");

    printf("idle pid: %d\n", (int)idle_pid);

    cpu_switch_context_exit();
}
