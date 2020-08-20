#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#include <dlfcn.h>
#else
#include <dlfcn.h>
#endif
#include "byteorder.h"
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

#include <vcrtos/instance.h>
#include <vcrtos/cpu.h>
#include <vcrtos/thread.h>

#include "native_internal.h"

void *_native_instance;

extern void kernel_init(void *instance);

const char *_progname;
char **_native_argv;
pid_t _native_pid;
pid_t _native_id;

typedef void (*init_func_t)(int argc, char **argv, char **envp);

extern init_func_t __init_array_start;
extern init_func_t __init_array_end;

static void _reset_handler(void)
{
    //cpu_reboot();
}

__attribute__((constructor)) static void startup(int argc, char **argv, char **envp)
{
    _native_init_syscalls();
    //stdio_init();

    _native_argv = argv;
    _progname = argv[0];
    _native_pid = real_getpid();

    _native_id = _native_pid;

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

    native_cpu_init();
    native_interrupt_init();

    _native_instance = instance_init_single();

    puts("native hardware initialization complete.\n");

    cpu_irq_enable();

    extern void kernel_init(void *);
    kernel_init(_native_instance);
}
