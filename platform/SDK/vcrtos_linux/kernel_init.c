/*
 * Copyright (c) 2020, Vertexcom Technologies, Inc.
 * All rights reserved.
 *
 * NOTICE: All information contained herein is, and remains
 * the property of Vertexcom Technologies, Inc. and its suppliers,
 * if any. The intellectual and technical concepts contained
 * herein are proprietary to Vertexcom Technologies, Inc.
 * and may be covered by U.S. and Foreign Patents, patents in process,
 * and protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Vertexcom Technologies, Inc.
 *
 * Authors: Darko Pancev <darko.pancev@vertexcom.com>
 */

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

#include "native_internal.h"

#if VCRTOS_CONFIG_ZTIMER_ENABLE
#define WIDTH_TO_MAXVAL(width) (UINT32_MAX >> (32 - width))
static ztimer_periph_timer_t _ztimer_periph_timer_usec = {
    .min = VCRTOS_CONFIG_ZTIMER_USEC_MIN
};
ztimer_clock_t *const ZTIMER_USEC = &_ztimer_periph_timer_usec.super;
#endif

extern int main(void);

static void *thread_main_handler(void *arg)
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

static void *thread_idle_handler(void *arg)
{
    (void) arg;

    while (1)
    {
        native_cpu_sleep();
    }

    return NULL;
}

static char _main_stack[VCRTOS_CONFIG_MAIN_THREAD_STACK_SIZE];
static char _idle_stack[VCRTOS_CONFIG_IDLE_THREAD_STACK_SIZE];

#if VCRTOS_CONFIG_THREAD_EVENT_ENABLE
static void *_thread_event_handler(void *arg)
{
    event_queue_t *queue = (event_queue_t *)arg;
    event_queue_claim(queue);
    event_loop(queue);
    return NULL;
}

event_queue_t event_queue_highest;
static char _event_queue_highest_stack[VCRTOS_CONFIG_THREAD_EVENT_HIGHEST_STACK_SIZE];

event_queue_t event_queue_medium;
static char _event_queue_medium_stack[VCRTOS_CONFIG_THREAD_EVENT_MEDIUM_STACK_SIZE];

event_queue_t event_queue_lowest;
static char _event_queue_lowest_stack[VCRTOS_CONFIG_THREAD_EVENT_LOWEST_STACK_SIZE];

typedef struct
{
    event_queue_t *queue;
    char *stack;
    size_t stack_size;
    unsigned priority;
} event_threads_t;

const event_threads_t _event_threads[] = {
    { &event_queue_highest,
      _event_queue_highest_stack,
      sizeof(_event_queue_highest_stack),
      VCRTOS_CONFIG_THREAD_EVENT_HIGHEST_PRIORITY
    },
    { &event_queue_medium,
      _event_queue_medium_stack,
      sizeof(_event_queue_medium_stack),
      VCRTOS_CONFIG_THREAD_EVENT_MEDIUM_PRIORITY
    },
    { &event_queue_lowest,
      _event_queue_lowest_stack,
      sizeof(_event_queue_lowest_stack),
      VCRTOS_CONFIG_THREAD_EVENT_LOWEST_PRIORITY
    }
};

void event_thread_init(void *instance,
                       event_queue_t *queue,
                       char *stack,
                       size_t stack_size,
                       unsigned priority)
{
    event_queue_init(instance, queue);

    (void) thread_create(instance, stack, stack_size, priority,
                THREAD_FLAGS_CREATE_WOUT_YIELD | THREAD_FLAGS_CREATE_STACKMARKER,
                _thread_event_handler, (void *)queue, "event");
}
#endif

void _kernel_init(void *instance)
{
    (void) cpu_irq_disable();

    vcassert(instance_is_initialized(instance));

    real_printf("\r\n\r\nvcrtos-%s kernel started\r\n\r\n", VCRTOS_VERSION);

    (void) thread_create((void *)instance, _main_stack, sizeof(_main_stack),
                         KERNEL_THREAD_PRIORITY_MAIN,
                         THREAD_FLAGS_CREATE_WOUT_YIELD | THREAD_FLAGS_CREATE_STACKMARKER,
                         thread_main_handler, (void *)instance, "main");

    (void) thread_create((void *)instance, _idle_stack, sizeof(_idle_stack),
                         KERNEL_THREAD_PRIORITY_IDLE,
                         THREAD_FLAGS_CREATE_WOUT_YIELD | THREAD_FLAGS_CREATE_STACKMARKER,
                         thread_idle_handler, (void *)instance, "idle");

#if VCRTOS_CONFIG_ZTIMER_ENABLE
    ztimer_periph_timer_init(&_ztimer_periph_timer_usec,
                             VCRTOS_CONFIG_ZTIMER_USEC_DEV,
                             VCRTOS_CONFIG_ZTIMER_USEC_BASE_FREQ,
                             WIDTH_TO_MAXVAL(VCRTOS_CONFIG_ZTIMER_USEC_WIDTH));
#endif

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
