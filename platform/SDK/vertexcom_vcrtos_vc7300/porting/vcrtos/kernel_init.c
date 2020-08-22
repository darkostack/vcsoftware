#include <stdlib.h>
#include <stdio.h>

#include <vcdrivers/stdiobase.h>

#include <vcrtos/config.h>
#include <vcrtos/assert.h>
#include <vcrtos/cpu.h>
#include <vcrtos/instance.h>
#include <vcrtos/thread.h>

#if VCRTOS_CONFIG_ZTIMER_ENABLE
#include <vcrtos/ztimer.h>
#include <vcrtos/ztimer/periph_timer.h>
#endif

#if VCRTOS_CONFIG_THREAD_EVENT_ENABLE
#include <vcrtos/event.h>
#endif

#if VCRTOS_CONFIG_ZTIMER_ENABLE
#define WIDTH_TO_MAXVAL(width) (UINT32_MAX >> (32 - width))
static ztimer_periph_timer_t _ztimer_periph_timer_usec = {
    .min = VCRTOS_CONFIG_ZTIMER_USEC_MIN
};
ztimer_clock_t *const ZTIMER_USEC = &_ztimer_periph_timer_usec.super;
#endif

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
    (void) arg;

    while (1)
    {
        cpu_sleep(0);
    }

    return NULL;
}

char _main_stack[VCRTOS_CONFIG_MAIN_THREAD_STACK_SIZE];
char _idle_stack[VCRTOS_CONFIG_IDLE_THREAD_STACK_SIZE];

#if VCRTOS_CONFIG_THREAD_EVENT_ENABLE

void *thread_event_handler(void *arg)
{
    event_queue_t *queue = (event_queue_t *)arg;

    event_queue_claim(queue);
    event_loop(queue);

    return NULL;
}

event_queue_t event_queue_highest;
char event_queue_highest_stack[VCRTOS_CONFIG_THREAD_EVENT_HIGHEST_STACK_SIZE];

event_queue_t event_queue_medium;
char event_queue_medium_stack[VCRTOS_CONFIG_THREAD_EVENT_MEDIUM_STACK_SIZE];

event_queue_t event_queue_lowest;
char event_queue_lowest_stack[VCRTOS_CONFIG_THREAD_EVENT_LOWEST_STACK_SIZE];

typedef struct
{
    event_queue_t *queue;
    char *stack;
    size_t stack_size;
    unsigned priority;
} event_threads_t;

const event_threads_t _event_threads[] = {
    { &event_queue_highest, event_queue_highest_stack, sizeof(event_queue_highest_stack),
      VCRTOS_CONFIG_THREAD_EVENT_HIGHEST_PRIORITY },
    { &event_queue_medium, event_queue_medium_stack, sizeof(event_queue_medium_stack),
      VCRTOS_CONFIG_THREAD_EVENT_MEDIUM_PRIORITY },
    { &event_queue_lowest, event_queue_lowest_stack, sizeof(event_queue_lowest_stack),
      VCRTOS_CONFIG_THREAD_EVENT_LOWEST_PRIORITY }
};

void event_thread_init(void *instance, event_queue_t *queue, char *stack, size_t stack_size, unsigned priority)
{
    event_queue_init(instance, queue);

    (void) thread_create(instance, stack, stack_size, priority,
                         THREAD_FLAGS_CREATE_WOUT_YIELD | THREAD_FLAGS_CREATE_STACKMARKER,
                         thread_event_handler, queue, "event");
}

#endif // #if VCRTOS_CONFIG_THREAD_EVENT_ENABLE

void kernel_init(void)
{
    (void) cpu_irq_disable();

    instance_t *instance = instance_init_single();

    vcassert(instance_is_initialized(instance));

    vcstdio_init(instance);

#if VCRTOS_CONFIG_ZTIMER_ENABLE
    ztimer_periph_timer_init(&_ztimer_periph_timer_usec,
                             VCRTOS_CONFIG_ZTIMER_USEC_DEV,
                             VCRTOS_CONFIG_ZTIMER_USEC_BASE_FREQ,
                             WIDTH_TO_MAXVAL(VCRTOS_CONFIG_ZTIMER_USEC_WIDTH));
#endif

    printf("\r\n\r\nvcrtos-%s kernel started\r\n\r\n", VCRTOS_VERSION);

    (void) thread_create((void *)instance, _main_stack, sizeof(_main_stack),
                         KERNEL_THREAD_PRIORITY_MAIN,
                         THREAD_FLAGS_CREATE_WOUT_YIELD | THREAD_FLAGS_CREATE_STACKMARKER,
                         thread_main_handler, (void *)instance, "main");

    (void) thread_create((void *)instance, _idle_stack, sizeof(_idle_stack),
                         KERNEL_THREAD_PRIORITY_IDLE,
                         THREAD_FLAGS_CREATE_WOUT_YIELD | THREAD_FLAGS_CREATE_STACKMARKER,
                         thread_idle_handler, (void *)instance, "idle");

#if VCRTOS_CONFIG_THREAD_EVENT_ENABLE
    for (unsigned i = 0; i < (sizeof(_event_threads) / sizeof(_event_threads[0])); i++)
    {
        event_thread_init(instance, _event_threads[i].queue,
                          _event_threads[i].stack, _event_threads[i].stack_size,
                          _event_threads[i].priority);
    }
#endif

    cpu_switch_context_exit();
}
