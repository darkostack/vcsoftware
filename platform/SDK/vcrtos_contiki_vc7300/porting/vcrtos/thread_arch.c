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
#include <vcrtos/instance.h>
#include <vcrtos/thread.h>

void _thread_exit(void)
{
    thread_exit((void *)instance_get());
}

char *thread_arch_stack_init(thread_handler_func_t func, void *arg, void *stack_start, int stack_size)
{
    uint32_t *stk;
    stk = (uint32_t *)((uintptr_t)stack_start + stack_size);

    /* adjust to 32 bit boundary by clearing the last two bits in the address */
    stk = (uint32_t *)(((uint32_t)stk) & ~((uint32_t)0x3));

    /* stack start marker */
    stk--;
    *stk = STACK_MARKER;

    /* make sure the stack is double word aligned (8 bytes) */
    /* This is required in order to conform with Procedure Call Standard for the
     * ARM® Architecture (AAPCS) */
    /* http://infocenter.arm.com/help/topic/com.arm.doc.ihi0042e/IHI0042E_aapcs.pdf */
    if (((uint32_t) stk & 0x7) != 0) {
        /* add a single word padding */
        --stk;
        *stk = ~((uint32_t)STACK_MARKER);
    }

    /* ****************************** */
    /* Automatically popped registers */
    /* ****************************** */

    /* The following eight stacked registers are popped by the hardware upon
     * return from exception. (bx instruction in select_and_restore_context) */

    /* xPSR - initial status register */
    stk--;
    *stk = (uint32_t)INITIAL_XPSR;
    /* pc - initial program counter value := thread entry function */
    stk--;
    *stk = (uint32_t)func;
    /* lr - contains the return address when the thread exits */
    stk--;
    *stk = (uint32_t)_thread_exit;
    /* r12 */
    stk--;
    *stk = 0;
    /* r3 - r1 */
    for (int i = 3; i >= 1; i--) {
        stk--;
        *stk = i;
    }
    /* r0 - contains the thread function parameter */
    stk--;
    *stk = (uint32_t)arg;

    /* ************************* */
    /* Manually popped registers */
    /* ************************* */

    /* The following registers are not handled by hardware in return from
     * exception, but manually by select_and_restore_context.
     * For the Cortex-M0(plus) we write registers R11-R4 in two groups to allow
     * for more efficient context save/restore code.
     * For the Cortex-M3 and Cortex-M4 we write them continuously onto the stack
     * as they can be read/written continuously by stack instructions. */

    /* r11 - r4 */
    for (int i = 11; i >= 4; i--) {
        stk--;
        *stk = i;
    }

    /* exception return code  - return to task-mode process stack pointer */
    stk--;
    *stk = (uint32_t)EXCEPT_RET_TASK_MODE;

    /* The returned stack pointer will be aligned on a 32 bit boundary not on a
     * 64 bit boundary because of the odd number of registers above (8+9).
     * This is not a problem since the initial stack pointer upon process entry
     * _will_ be 64 bit aligned (because of the cleared bit 9 in the stacked
     * xPSR and aligned stacking of the hardware-handled registers). */

    return (char*)stk;
}

void thread_arch_stack_print(void)
{
    int count = 0;
    thread_t *current_thread = thread_current(instance_get());
    uint32_t *sp = (uint32_t *)current_thread->stack_pointer;

    printf("printing the current stack of thread %" PRIkernel_pid "\n", current_thread->pid);
    printf("  address:      data:\n");

    do {
        printf("  0x%08x:   0x%08x\n", (unsigned int)sp, (unsigned int)*sp);
        sp++;
        count++;
    } while (*sp != STACK_MARKER);

    printf("current stack size: %i byte\n", count);
}

int thread_arch_isr_stack_usage(void)
{
    uint32_t *ptr = &_sstack;

    while (((*ptr) == STACK_CANARY_WORD) && (ptr < &_estack))
    {
        ++ptr;
    }

    ptrdiff_t num_used_words = &_estack - ptr;

    return num_used_words * sizeof(*ptr);
}

void *thread_arch_isr_stack_pointer(void)
{
    void *msp = (void *)__get_MSP();
    return msp;
}

void *thread_arch_isr_stack_start(void)
{
    return (void *)&_sstack;
}

__attribute__((naked)) __attribute__((used)) void isr_pendsv(void)
{
    __asm__ volatile(
        /* PendSV handler entry point */
        /* save context by pushing unsaved registers to the stack */
        /* {r0-r3,r12,LR,PC,xPSR,s0-s15,FPSCR} are saved automatically on exception entry */
        ".thumb_func                            \n"
        "mrs    r1, psp                         \n" /* get stack pointer from user mode */
        "stmdb  r1!,{r4-r11}                    \n" /* save regs */
        "stmdb  r1!,{lr}                        \n" /* exception return value */
        "bl instance_get                        \n" /* get instance ptr and return value stored in r0 */
        "bl thread_current                      \n" /* get current TCB and return value is stored in r0 */
        "str    r1, [r0]                        \n" /* write r1 to TCB->sp */
        "bl     isr_svc                         \n" /* continue with svc */
    );

}

__attribute__((naked)) __attribute__((used)) void isr_svc(void)
{
    __asm__ volatile(
        /* SVC handler entry point */
        /* PendSV will continue here as well (via jump) */
        ".thumb_func                        \n"
        /* perform scheduling */
        "bl instance_get                    \n"
        "bl thread_scheduler_run            \n"
        /* restore context and return from exception */
        ".thumb_func                        \n"
        "context_restore:                   \n"
        "bl instance_get                    \n" /* get single instance ptr and return value stored in r0 */
        "bl thread_current                  \n" /* get current TCB and return value is stored in r0 */
        "ldr    r1, [r0]                    \n" /* load TCB->sp to register 1 */
        "ldmia  r1!, {r0}                   \n" /* restore exception return value */
        "ldmia  r1!, {r4-r11}               \n" /* restore other registers */
        "msr    psp, r1                     \n" /* restore user mode SP to PSP reg */
        "bx     r0                          \n" /* load exception return value to PC,
                                                 * causes end of exception*/
        /* {r0-r3,r12,LR,PC,xPSR,s0-s15,FPSCR} are restored automatically on exception return */
    );
}

void thread_arch_yield_higher(void)
{
    cpu_trigger_pendsv_interrupt();
}
