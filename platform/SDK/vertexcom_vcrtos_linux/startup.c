#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#include <dlfcn.h>
#else
#include <dlfcn.h>
#endif
#include <assert.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <vcdrivers/config.h>
#include <vcrtos/config.h>

#include <vcrtos/instance.h>
#include <vcrtos/cpu.h>
#include <vcrtos/thread.h>

#include "native_internal.h"
#include "tty_uart.h"

void *_native_instance;

typedef enum {
    _STDIOTYPE_STDIO = 0,
    _STDIOTYPE_NULL,
    _STDIOTYPE_FILE,
} _stdiotype_t;

int _native_null_in_pipe[2];
int _native_null_out_file;
const char *_progname;
char **_native_argv;
pid_t _native_pid;
pid_t _native_id;

static const char short_opts[] = ":hi:eEoc:";

static const struct option long_opts[] = {
    { "help", no_argument, NULL, 'h' },
    { "id", required_argument, NULL, 'i' },
    { "stderr-pipe", no_argument, NULL, 'e' },
    { "stderr-noredirect", no_argument, NULL, 'E' },
    { "stdout-pipe", no_argument, NULL, 'o' },
    { "uart-tty", required_argument, NULL, 'c' },
    { NULL, 0, NULL, '\0' },
};

void _native_input(_stdiotype_t stdintype)
{
    if (real_pipe(_native_null_in_pipe) == -1)
    {
        err(EXIT_FAILURE, "_native_null_in(): pipe");
    }

    if (stdintype == _STDIOTYPE_STDIO)
    {
        return;
    }

    if (real_dup2(_native_null_in_pipe[0], STDIN_FILENO) == -1)
    {
        err(EXIT_FAILURE, "_native_null_in: dup2(STDIN_FILENO)");
    }
}

int _native_log_output(_stdiotype_t stdiotype, int output)
{
    int outfile;

    switch (stdiotype)
    {
    case _STDIOTYPE_STDIO:
        return -1;
    case _STDIOTYPE_NULL:
        if ((outfile = real_open("/dev/null", O_WRONLY)) == -1)
        {
            err(EXIT_FAILURE, "_native_log_output: open");
        }
        break;
    case _STDIOTYPE_FILE: {
        char logname[sizeof("/tmp/vcrtos.stderr.") + 20];
        snprintf(logname, sizeof(logname), "/tmp/vcrtos.std%s.%d",
                 (output == STDOUT_FILENO) ? "out": "err", _native_pid);
        if ((outfile = real_creat(logname, 0666)) == -1)
        {
            err(EXIT_FAILURE, "_native_log_output: open");
        }
        break;
    }
    default:
        errx(EXIT_FAILURE, "_native_log_output: unknown log type");
        break;
    }
    if (real_dup2(outfile, output) == -1)
    {
        err(EXIT_FAILURE, "_native_log_output: dup2(output)");
    }
    return outfile;
}

void usage_exit(int status)
{
    real_printf("usage: %s", _progname);
    real_printf(" [-i <id>] [-e|E] [-o] [-c <tty>]\n"); 

    real_printf(" help: %s -h\n\n", _progname);

    real_printf("\nOptions:\n"
"     -h, --help\n"
"         print this help message\n"
"     -i <id>, --id=<id>\n"
"         specify instance id\n"
"     -e, --stderr-pipe\n"
"         redirect stderr to file\n"
"     -E, --stderr-noredirect\n"
"         do not redirect stderr\n"
"     -o, --stdout-pipe\n"
"         redirect stdout to file (/tmp/vcrtos.stdiout.PID)\n"
"     -c <tty>, --uart-tty=<tty>\n"
"         specify TTY device for UART. This argument can be used multiple\n"
"         times (up to UART_NUMOF)\n"
    );

    real_exit(status);
}

typedef void (*init_func_t)(int argc, char **argv, char **envp);

extern init_func_t __init_array_start;
extern init_func_t __init_array_end;

static void _reset_handler(void)
{
    real_printf("\n\nREBOOT\n\n");

    if (real_execve(_native_argv[0], _native_argv, NULL) == -1)
    {
        err(EXIT_FAILURE, "reset_handler: execve");
    }

    errx(EXIT_FAILURE, "reset_handler: this should not have been reached");
}

__attribute__((constructor)) static void startup(int argc, char **argv, char **envp)
{
    _native_init_syscalls();

    _native_argv = argv;
    _progname = argv[0];
    _native_pid = real_getpid();

    /* will possible be overridden via option below */
    _native_id = _native_pid;

    int c, opt_idx = 0;
    bool force_stderr = false;

    _stdiotype_t stderrtype = _STDIOTYPE_STDIO;
    _stdiotype_t stdouttype = _STDIOTYPE_STDIO;
    _stdiotype_t stdintype = _STDIOTYPE_STDIO;

    while ((c = getopt_long(argc, argv, short_opts, long_opts, &opt_idx)) >= 0)
    {
        switch (c)
        {
        case 0:
        case 'h':
            usage_exit(EXIT_SUCCESS);
            break;
        case 'i':
            _native_id = atol(optarg);
            break;
        case 'e':
            if (force_stderr)
            {
                /* -e and -E are mutually exclusive */
                usage_exit(EXIT_FAILURE);
            }
            stderrtype = _STDIOTYPE_FILE;
            break;
        case 'E':
            if (stderrtype == _STDIOTYPE_FILE)
            {
                /* -e and -E are mutually exclusive */
                usage_exit(EXIT_FAILURE);
            }
            force_stderr = true;
            break;
        case 'o':
            stdouttype = _STDIOTYPE_FILE;
            break;
        case 'c':
            tty_uart_setup(VCDRIVERS_CONFIG_STDIOBASE_UART_DEV, optarg);
            break;
        default:
            usage_exit(EXIT_FAILURE);
            break;
        }
    }

    _native_log_output(stderrtype, STDERR_FILENO);
    _native_null_out_file = _native_log_output(stdouttype, STDOUT_FILENO);
    _native_input(stdintype);

    /* startup is a constructor which is being called from the init_array during
     * C runtime initialization, this is normally used for code which must run
     * before launching main(), such as C++ global object constructors etc.
     * However, this function (startup) misbehaves a bit when we call
     * kernel_init below, which does not return until there is an abort or a
     * power off command.
     * We need all C++ global constructors and other initializers to run before
     * we enter the normal application code, which may depend on global objects
     * having been initialized properly. Therefore, we iterate through the
     * remainder of the init_array and call any constructors which have been
     * placed after startup in the initialization order.
     */

    init_func_t *init_array_ptr = &__init_array_start;

    while (init_array_ptr != &__init_array_end)
    {
        /* Skip everything which has already been run */
        if ((*init_array_ptr) == startup)
        {
            /* Found ourselves, move on to calling the rest of the constructors */
            ++init_array_ptr;
            break;
        }
        ++init_array_ptr;
    }
    while (init_array_ptr != &__init_array_end)
    {
        /* call all remaining constructors */
        (*init_array_ptr)(argc, argv, envp);
        ++init_array_ptr;
    }

    _native_instance = instance_init_single();

    native_cpu_init();
    native_interrupt_init();

    register_interrupt(SIGUSR1, _reset_handler);

    //real_printf("native %d hardware initialization complete.\n", (int)_native_id);

    cpu_irq_enable();

    extern void _kernel_init(void *);
    _kernel_init(_native_instance);
}
