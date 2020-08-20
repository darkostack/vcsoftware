#include <err.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define __USE_GNU
#include <signal.h>
#undef __USE_GNU

#include <vcrtos/cpu.h>
#include <vcrtos/thread.h>
#include <vcrtos/instance.h>

#include "native_internal.h"

volatile int native_interrupts_enabled = 0;
volatile int _native_in_isr;
volatile int _native_in_syscall;

static sigset_t _native_sig_set, _native_sig_set_dint;

char __isr_stack[SIGSTKSZ];
ucontext_t native_isr_context;
ucontext_t *_native_cur_ctx, *_native_isr_ctx;

volatile unsigned int _native_saved_eip;
volatile int _native_sigpend;
int _sig_pipefd[2];

static _native_callback_t native_irq_handlers[255];
char sigalt_stk[SIGSTKSZ];

void *thread_arch_isr_stack_pointer(void)
{
    return native_isr_context.uc_stack.ss_sp;
}

void *thread_arch_isr_stack_start(void)
{
    return __isr_stack;
}

void print_thread_sigmask(ucontext_t *cp)
{
    sigset_t *p = &cp->uc_sigmask;

    if (sigemptyset(p) == -1)
    {
        err(EXIT_FAILURE, "print_thread_sigmask: sigemptyset");
    }

    for (int i = 1; i < (NSIG); i++)
    {
        if (native_irq_handlers[i] != NULL)
        {
            printf("%s: %s\n", strsignal(i), (sigismember(&_native_sig_set, i) ? "blocked" : "unblocked"));
        }

        if (sigismember(p, i))
        {
            printf("%s: pending\n", strsignal(i));
        }
    }
}

void print_sigmask(void)
{
    for (int i = 0; i < KERNEL_MAXTHREADS; i++)
    {
        if (thread_get_from_scheduler(_native_instance, i) != NULL)
        {
            ucontext_t *p;
            printf("%s:\n", thread_get_from_scheduler(_native_instance, i)->name);
            p = (ucontext_t *)(thread_get_from_scheduler(_native_instance, i)->stack_start);
            print_thread_sigmask(p);
            puts("");
        }
    }
}

void native_print_signals(void)
{
    sigset_t p, q;

    puts("native signals:\n");

    if (sigemptyset(&p) == -1)
    {
        err(EXIT_FAILURE, "native_print_signals: sigemptyset");
    }

    if (sigpending(&p) == -1)
    {
        err(EXIT_FAILURE, "native_print_signals: sigpending");
    }

    if (sigprocmask(SIG_SETMASK, NULL, &q) == -1)
    {
        err(EXIT_FAILURE, "native_print_signals: sigprocmask");
    }

    for (int i = 1; i < (NSIG); i++)
    {
        if (native_irq_handlers[i] != NULL || i == SIGUSR1)
        {
            printf("%s: %s in active thread\n",
                   strsignal(i),
                   (sigismember(&_native_sig_set, i) ? "blocked" : "unblocked"));
        }

        if (sigismember(&p, i))
        {
            printf("%s: pending\n", strsignal(i));
        }

        if (sigismember(&q, i))
        {
            printf("%s: blocked in this context\n", strsignal(i));
        }
    }
}

/* block signals */
unsigned cpu_irq_disable(void)
{
    unsigned int prev_state;

    _native_syscall_enter();

    if (sigprocmask(SIG_SETMASK, &_native_sig_set_dint, NULL) == -1)
    {
        err(EXIT_FAILURE, "irq_disable: sigprocmask");
    }

    prev_state = native_interrupts_enabled;
    native_interrupts_enabled = 0;

    _native_syscall_leave();

    return prev_state;
}

/* unblock signals */
unsigned cpu_irq_enable(void)
{
    unsigned int prev_state;

    _native_syscall_enter();

    prev_state = native_interrupts_enabled;
    native_interrupts_enabled = 1;

    if (sigprocmask(SIG_SETMASK, &_native_sig_set, NULL) == -1)
    {
        err(EXIT_FAILURE, "irq_enable: sigprocmask");
    }

    _native_syscall_leave();

    return prev_state;
}

void cpu_irq_restore(unsigned state)
{
    if (state == 1)
    {
        cpu_irq_enable();
    }
    else
    {
        cpu_irq_disable();
    }

    return;
}

int cpu_is_in_isr(void)
{
    return _native_in_isr;
}

int _native_popsig(void)
{
    int nread, nleft, i;
    int sig = 0;

    nleft = sizeof(int);
    i = 0;

    while ((nleft > 0) && ((nread = real_read(_sig_pipefd[0], ((uint8_t *)&sig) + i, nleft)) != -1))
    {
        i += nread;
        nleft -= nread;
    }

    if (nread == -1)
    {
        err(EXIT_FAILURE, "_native_popsig: real_read");
    }

    return sig;
}

/* call signal handlers, restore user context */
void native_irq_handler(void)
{
    while (_native_sigpend > 0)
    {
        int sig = _native_popsig();
        _native_sigpend--;

        if (native_irq_handlers[sig] != NULL)
        {
            native_irq_handlers[sig]();
        }
        else if (sig == SIGUSR1)
        {
            warnx("native_irq_handler: ignoring SIGUSR1");
        }
        else
        {
            err(EXIT_FAILURE, "XXX: no handler for signal %i\nXXX: this should not have happened!\n", sig);
        }
    }

    cpu_switch_context_exit();
}

void isr_set_sigmask(ucontext_t *ctx)
{
    ctx->uc_sigmask = _native_sig_set_dint;
    native_interrupts_enabled = 0;
}

void native_isr_entry(int sig, siginfo_t *info, void *context)
{
    (void) info;

    if (real_write(_sig_pipefd[1], &sig, sizeof(int)) == -1)
    {
        err(EXIT_FAILURE, "native_isr_entry: real_write");
    }

    _native_sigpend++;

    if (context == NULL)
    {
        errx(EXIT_FAILURE, "native_isr_entry: context is null - unhandled");
    }

    if (thread_current(_native_instance) == NULL)
    {
        _native_in_isr++;
        warnx("native_isr_entry: current active thread is null - unhandled");
        _native_in_isr--;
        return;
    }

    if (native_interrupts_enabled == 0)
    {
        return;
    }

    if (_native_in_syscall != 0)
    {
        return;
    }

    native_isr_context.uc_stack.ss_sp = __isr_stack;
    native_isr_context.uc_stack.ss_size = sizeof(__isr_stack);
    native_isr_context.uc_stack.ss_flags = 0;
    makecontext(&native_isr_context, native_irq_handler, 0);
    _native_cur_ctx = (ucontext_t *)thread_current(_native_instance)->stack_pointer;

    isr_set_sigmask((ucontext_t *)context);
    _native_in_isr = 1;

    _native_saved_eip = ((ucontext_t *)context)->uc_mcontext.gregs[REG_EIP];
    ((ucontext_t *)context)->uc_mcontext.gregs[REG_EIP] = (unsigned int)&_native_sig_leave_tramp;
}

void set_signal_handler(int sig, bool add)
{
    struct sigaction sa;
    int ret;

    if (add)
    {
        _native_syscall_enter();
        ret = sigdelset(&_native_sig_set, sig);
        _native_syscall_leave();
    }
    else
    {
        _native_syscall_enter();
        ret = sigaddset(&_native_sig_set, sig);
        _native_syscall_leave();
    }

    if (ret == -1)
    {
        err(EXIT_FAILURE, "set_signal_handler: sigdelset");
    }

    memset(&sa, 0, sizeof(sa));

    memcpy(&sa.sa_mask, &_native_sig_set_dint, sizeof(sa.sa_mask));

    sa.sa_flags = SA_RESTART | SA_ONSTACK;

    if (add)
    {
        sa.sa_flags |= SA_SIGINFO;
        sa.sa_sigaction = native_isr_entry;
    }
    else
    {
        sa.sa_handler = SIG_IGN;
    }

    _native_syscall_enter();
    if (sigaction(sig, &sa, NULL))
    {
        err(EXIT_FAILURE, "set_signal_handler: sigaction");
    }
    _native_syscall_leave();
}

int register_interrupt(int sig, _native_callback_t handler)
{
    unsigned state = cpu_irq_disable();

    native_irq_handlers[sig] = handler;
    set_signal_handler(sig, true);

    cpu_irq_restore(state);

    return 0;
}

int unregister_interrupt(int sig)
{
    unsigned state = cpu_irq_disable();

    set_signal_handler(sig, false);
    native_irq_handlers[sig] = NULL;

    cpu_irq_restore(state);

    return 0;
}

static void native_shutdown(int sig, siginfo_t *info, void *context)
{
    (void) sig;
    (void) info;
    (void) context;

    real_exit(EXIT_SUCCESS);
}

void native_interrupt_init(void)
{
    struct sigaction sa;

    _native_sigpend = 0;

    for (int i = 0; i < 255; i++)
    {
        native_irq_handlers[i] = NULL;
    }

    sa.sa_sigaction = native_isr_entry;

    if (sigfillset(&sa.sa_mask) == -1)
    {
        err(EXIT_FAILURE, "native_interrupt_init: sigfillset");
    }

    sa.sa_flags = SA_RESTART | SA_SIGINFO | SA_ONSTACK;

    if (sigfillset(&_native_sig_set) == -1)
    {
        err(EXIT_FAILURE, "native_interrupt_init: sigfillset");
    }

    if (sigfillset(&_native_sig_set_dint) == -1)
    {
        err(EXIT_FAILURE, "native_interrupt_init: sigfillset");
    }

    if (sigdelset(&_native_sig_set, SIGUSR1) == -1)
    {
        err(EXIT_FAILURE, "native_interrupt_init: sigdelset");
    }
    if (sigdelset(&_native_sig_set_dint, SIGUSR1) == -1)
    {
        err(EXIT_FAILURE, "native_interrupt_init: sigdelset");
    }

    if (sigaction(SIGUSR1, &sa, NULL))
    {
        err(EXIT_FAILURE, "native_interrupt_init: sigaction");
    }

    if (getcontext(&native_isr_context) == -1)
    {
        err(EXIT_FAILURE, "native_interrupt_init: getcontext");
    }

    native_isr_context.uc_stack.ss_sp = __isr_stack;
    native_isr_context.uc_stack.ss_size = sizeof(__isr_stack);
    native_isr_context.uc_stack.ss_flags = 0;
    _native_isr_ctx = &native_isr_context;

    static stack_t sigstk;
    sigstk.ss_sp = sigalt_stk;
    sigstk.ss_size = sizeof(__isr_stack);
    sigstk.ss_flags = 0;

    if (sigaltstack(&sigstk, NULL) < 0)
    {
        err(EXIT_FAILURE, "native_interrupt_init: sigaltstack");
    }

    makecontext(&native_isr_context, native_irq_handler, 0);

    _native_in_syscall = 0;

    if (real_pipe(_sig_pipefd) == -1)
    {
        err(EXIT_FAILURE, "native_interrupt_init: pipe");
    }

    sa.sa_sigaction = native_shutdown;
    if (sigdelset(&_native_sig_set, SIGINT) == -1)
    {
        err(EXIT_FAILURE, "native_interrupt_init: sigdelset");
    }
    if (sigdelset(&_native_sig_set_dint, SIGINT) == -1)
    {
        err(EXIT_FAILURE, "native_interrupt_init: sigdelset");
    }
    if (sigaction(SIGINT, &sa, NULL))
    {
        err(EXIT_FAILURE, "native_interrupt_init: sigaction");
    }

    puts("native interrupts/signals initialized.");
}
