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

#include <stdlib.h>
#include <stdio.h>

#include <vcrtos/config.h>
#include <vcrtos/cli.h>
#include <vcrtos/instance.h>
#include <vcrtos/kernel.h>
#include <vcrtos/thread.h>

#include <vcdrivers/cpu.h>

static void ps(void *instance)
{
    const char queued_name[] = {'_', 'Q'};
    int overall_stacksz = 0, overall_used = 0;

    printf("\tpid | "
           "%-21s| "
           "%-9sQ | pri "
           "| stack  ( used) | base addr  | current     "
           "| runtime  | switches"
           "\r\n",
           "name", "state");

    int isr_usage = thread_arch_isr_stack_usage();
    void *isr_start = thread_arch_isr_stack_start();
    void *isr_sp = thread_arch_isr_stack_pointer();

    printf("\t  - | isr-stack            | -        - |"
           "   - | %6i (%5i) | %10p | %10p\r\n",
           CPU_ISR_STACK_SIZE, isr_usage, isr_start, isr_sp);

    overall_stacksz += CPU_ISR_STACK_SIZE;

    if (isr_usage > 0)
    {
        overall_used += isr_usage;
    }

    uint64_t rt_sum = 0;

    kernel_pid_t i = KERNEL_PID_FIRST;

    for (i = KERNEL_PID_FIRST; i <= KERNEL_PID_LAST; i++)
    {
        thread_t *p = thread_get_from_scheduler(instance, i);

        if (p != NULL)
        {
            rt_sum += thread_get_runtime_ticks(instance, i);
        }
    }

    for (i = KERNEL_PID_FIRST; i <= KERNEL_PID_LAST; i++)
    {
        thread_t *p = thread_get_from_scheduler(instance, i);

        if (p != NULL)
        {
            thread_status_t state = p->status;
            const char *sname = thread_status_to_string(state);
            const char *queued = &queued_name[(int)(state >= THREAD_STATUS_RUNNING)];
            int stacksz = p->stack_size;

            overall_stacksz += stacksz;
            stacksz -= thread_measure_stack_free(p->stack_start);
            overall_used += stacksz;

            /* multiply with 100 for percentage and to avoid floats/doubles */
            uint64_t runtime_ticks = thread_get_runtime_ticks(instance, i) * 100;
            unsigned runtime_major = runtime_ticks / rt_sum;
            unsigned runtime_minor = ((runtime_ticks % rt_sum) * 1000) / rt_sum;
            unsigned switches = thread_get_schedules_stat(instance, i);

            printf("\t%3" PRIkernel_pid " | %-20s"
                   " | %-8s %.1s | %3i"
                   " | %6i (%5i) | %10p | %10p "
                   " | %2d.%03d%% |  %8u"
                   "\r\n",
                   p->pid, p->name, sname, queued, p->priority, p->stack_size, stacksz, (void *)p->stack_start,
                   (void *)p->stack_pointer, runtime_major, runtime_minor, switches);
        }
    }

    printf("\t%5s %-21s|%13s%6s %6i (%5i)\r\n", "|", "SUM", "|", "|", overall_stacksz, overall_used);
}

void vcrtos_cmd_ps(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    void *instance = instance_get();

    ps(instance);
}
