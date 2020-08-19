#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define __USE_GNU
#include <signal.h>
#undef __USE_GNU

#include <ucontext.h>
#include <err.h>

#include <stdlib.h>

#include <vcrtos/cpu.h>
#include <vcrtos/thread.h>

#include "native_internal.h"

ucontext_t end_context;
char __end_stack[SIGSTKSZ];

static void _native_mod_ctx_leave_sigh(ucontext_t *ctx)
{
    _native_saved_eip = ctx->uc_mcontext.gregs[REG_EIP];
    ctx->uc_mcontext.gregs[REG_EIP] = (unsigned int)&_native_sig_leave_handler;
}

void thread_arch_print_stack(void)
{
    return;
}

int thread_arch_isr_stack_usage(void)
{
    return -1;
}

char *thread_arch_stack_init(thread_handler_func_t func, void *arg, void *stack_start, int stacksize)
{
    char *stk;
    ucontext_t *p;

    stk = stack_start;

    p = (ucontext_t *)(stk + (stacksize - sizeof(ucontext_t)));
    stacksize -= sizeof(ucontext_t);

    if (getcontext(p) == -1)
    {
        err(EXIT_FAILURE, "thread_stack_init: getcontext");
    }

    p->uc_stack.ss_sp = stk;
    p->uc_stack.ss_size = stacksize;
    p->uc_stack.ss_flags = 0;
    p->uc_link = &end_context;

    if (sigemptyset(&(p->uc_sigmask)) == -1)
    {
        err(EXIT_FAILURE, "thread_stack_init: sigemptyset");
    }

    makecontext(p, (void (*)(void)) task_func, 1, arg);

    return (char *) p;
}

void isr_cpu_switch_context_exit(void)
{
    ucontext_t *ctx;

    if ((thread_scheduler_get_context_switch_request(_native_instance) == 1) ||
        (thread_current(_native_instance) == NULL))
    {
        thread_scheduler_run(_native_instance);
    }

    ctx = (ucontext_t *)(thread_current(_native_instance)->stack_pointer);

    native_interrupts_enabled = 1;
    _native_mod_ctx_leave_sigh(ctx);

    if (setcontext(ctx) == -1)
    {
        err(EXIT_FAILURE, "isr_cpu_switch_context_exit: setcontext");
    }

    errx(EXIT_FAILURE, "this should have never been reached!");
}

void cpu_switch_context_exit(void)
{
    if (_native_in_isr == 0)
    {
        cpu_irq_disable();
        _native_in_isr = 1;
        native_isr_context.uc_stack.ss_sp = __isr_stack;
        native_isr_context.uc_stack.ss_size = sizeof(__isr_stack);
        native_isr_context.uc_stack.ss_flags = 0;
        makecontext(&native_isr_context, isr_cpu_switch_context_exit, 0);
        if (setcontext(&native_isr_context) == -1)
        {
            err(EXIT_FAILURE, "cpu_switch_context_exit: setcontext");
        }
        errx(EXIT_FAILURE, "1 this should have never been reached!!");
    }
    else
    {
        isr_cpu_switch_context_exit();
    }
    errx(EXIT_FAILURE, "3 this should have never been reached!!");
}

void isr_thread_yield(void)
{
    if (_native_sigpend > 0)
    {
        native_irq_handler();
    }

    thread_scheduler_run(_native_instance);
    ucontext_t *ctx = (ucontext_t *)(thread_current(_native_instance)->stack_pointer);

    native_interrupts_enabled = 1;
    _native_mod_ctx_leave_sigh(ctx);

    if (setcontext(ctx) == -1)
    {
        err(EXIT_FAILURE, "isr_thread_yield: setcontext");
    }
}

void thread_arch_yield_higher(void)
{
    thread_scheduler_set_context_switch_request(_native_instance, 1);

    if (_native_in_isr == 0)
    {
        ucontext_t *ctx = (ucontext_t *)(thread_current(_native_instance)->stack_pointer);
        _native_in_isr = 1;
        if (!native_interrupts_enabled)
        {
            warnx("thread_yield_higher: interrupts are disabled - this should not be");
        }
        cpu_irq_disable();
        native_isr_context.uc_stack.ss_sp = __isr_stack;
        native_isr_context.uc_stack.ss_size = SIGSTKSZ;
        native_isr_context.uc_stack.ss_flags = 0;
        makecontext(&native_isr_context, isr_thread_yield, 0);
        if (swapcontext(ctx, &native_isr_context) == -1)
        {
            err(EXIT_FAILURE, "thread_yield_higher: swapcontext");
        }
        cpu_irq_enable();
    }
}

void sched_task_exit(void)
{
    thread_exit(_native_instance);
}

void native_cpu_init(void)
{
    if (getcontext(&end_context) == -1)
    {
        err(EXIT_FAILURE, "native_cpu_init: getcontext");
    }

    end_context.uc_stack.ss_sp = __end_stack;
    end_context.uc_stack.ss_size = SIGSTKSZ;
    end_context.uc_stack.ss_flags = 0;
    makecontext(&end_context, sched_task_exit, 0);
}
