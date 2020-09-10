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

#include <stdio.h>

#include <vcdrivers/cpu.h>

#include <vcrtos/cpu.h>
#include <vcrtos/thread.h>

unsigned cpu_irq_disable(void)
{
    unsigned mask = __get_PRIMASK();
    __disable_irq();
    return mask;
}

__attribute__((used)) unsigned cpu_irq_enable(void)
{
    __enable_irq();
    return __get_PRIMASK();
}

void cpu_irq_restore(unsigned state)
{
    __set_PRIMASK(state);
}

int cpu_is_in_isr(void)
{
    return (__get_IPSR() & 0xff);
}

void cpu_trigger_pendsv_interrupt(void)
{
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

__attribute__((naked)) void cpu_switch_context_exit(void)
{
    /* enable IRQs to make sure the SVC interrupt is reachable */
    cpu_irq_enable();

    /* trigger the SVC interrupt */
    __asm__ volatile (
        "svc    #1                            \n"
        : /* no outputs */
        : /* no inputs */
        : /* no clobbers */
    );

    /* should not reach here */

    while(1)
    {
    }
}

void cpu_print_last_instruction(void)
{
    uint32_t *lr_ptr;
    __asm__ __volatile__("mov %0, lr" : "=r"(lr_ptr));
    printf("%p\n", (void *)lr_ptr);
}

void cpu_sleep_until_event(void)
{
    __WFE();
}

void cpu_sleep(int deep)
{
    if (deep)
    {
        SCB->SCR |= (SCB_SCR_SLEEPDEEP_Msk);
    }
    else
    {
        SCB->SCR &= ~(SCB_SCR_SLEEPDEEP_Msk);
    }

    unsigned state = cpu_irq_disable();
    __DSB();
    __WFI();
    cpu_irq_restore(state);
}

void cpu_jump_to_image(uint32_t image_addr)
{
    __disable_irq();

    __set_MSP(*(uint32_t *)image_addr);

    image_addr += 4; /* skip stack pointer */

    uint32_t destination_addr = *(uint32_t *)image_addr;

    destination_addr |= 0x1; /* make sure thumb state bit is set */

    __asm("BX %0" ::"r"(destination_addr)); /* branch execution */
}

uint32_t cpu_get_image_base_addr(void)
{
    return SCB->VTOR;
}

void *cpu_get_msp(void)
{
    return (void *)__get_MSP();
}
