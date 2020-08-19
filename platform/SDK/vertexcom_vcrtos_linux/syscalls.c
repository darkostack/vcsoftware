#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#include <dlfcn.h>
#else
#include <dlfcn.h>
#endif

#include <err.h>
#include <errno.h>
#include <poll.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/time.h>
#include <sys/stat.h>

#include <vcrtos/cpu.h>
#include <vcrtos/thread.h>
#include <vcrtos/instance.h>
#include <vcrtos/xtimer.h>

#include "native_internal.h"

ssize_t (*real_read)(int fd, void *buf, size_t count);
ssize_t (*real_write)(int fd, const void *buf, size_t count);
size_t (*real_fread)(void *ptr, size_t size, size_t nmemb, FILE *stream);
void (*real_clearerr)(FILE *stream);
__attribute__((noreturn)) void (*real_exit)(int status);
void (*real_free)(void *ptr);
void* (*real_malloc)(size_t size);
void* (*real_calloc)(size_t nmemb, size_t size);
void* (*real_realloc)(void *ptr, size_t size);
void (*real_freeaddrinfo)(struct addrinfo *res);
void (*real_freeifaddrs)(struct ifaddrs *ifa);
void (*real_srandom)(unsigned int seed);
int (*real_accept)(int socket, ...);
int (*real_bind)(int socket, ...);
int (*real_connect)(int socket, ...);
int (*real_printf)(const char *format, ...);
int (*real_getaddrinfo)(const char *node, ...);
int (*real_getifaddrs)(struct ifaddrs **ifap);
int (*real_gettimeofday)(struct timeval *t, ...);
int (*real_getpid)(void);
int (*real_chdir)(const char *path);
int (*real_close)(int);
int (*real_fcntl)(int, int, ...);
int (*real_creat)(const char *path, ...);
int (*real_dup2)(int, int);
int (*real_execve)(const char *, char *const[], char *const[]);
int (*real_fork)(void);
int (*real_feof)(FILE *stream);
int (*real_ferror)(FILE *stream);
int (*real_listen)(int socket, int backlog);
int (*real_ioctl)(int fildes, int request, ...);
int (*real_open)(const char *path, int oflag, ...);
int (*real_pause)(void);
int (*real_pipe)(int[2]);
int (*real_select)(int nfds, ...);
int (*real_poll)(struct pollfd *fds, ...);
int (*real_setitimer)(int which, const struct itimerval
        *restrict value, struct itimerval *restrict ovalue);
int (*real_setsid)(void);
int (*real_setsockopt)(int socket, ...);
int (*real_socket)(int domain, int type, int protocol);
int (*real_unlink)(const char *);
long int (*real_random)(void);
const char* (*real_gai_strerror)(int errcode);
FILE* (*real_fopen)(const char *path, const char *mode);
int (*real_fclose)(FILE *stream);
int (*real_fseek)(FILE *stream, long offset, int whence);
int (*real_fputc)(int c, FILE *stream);
int (*real_fgetc)(FILE *stream);
mode_t (*real_umask)(mode_t cmask);
ssize_t (*real_writev)(int fildes, const struct iovec *iov, int iovcnt);

int (*real_clock_gettime)(clockid_t clk_id, struct timespec *tp);

void _native_syscall_enter(void)
{
    _native_in_syscall++;
}

void _native_syscall_leave(void)
{
    _native_in_syscall--;
    if (
            (_native_sigpend > 0)
            && (_native_in_isr == 0)
            && (_native_in_syscall == 0)
            && (native_interrupts_enabled == 1)
            && (thread_current(_native_instance) != NULL)
        )
    {
        _native_in_isr = 1;
        _native_cur_ctx = (ucontext_t *)thread_current(_native_instance)->stack_pointer;
        native_isr_context.uc_stack.ss_sp = __isr_stack;
        native_isr_context.uc_stack.ss_size = SIGSTKSZ;
        native_isr_context.uc_stack.ss_flags = 0;
        native_interrupts_enabled = 0;
        makecontext(&native_isr_context, native_irq_handler, 0);
        if (swapcontext(_native_cur_ctx, &native_isr_context) == -1)
        {
            err(EXIT_FAILURE, "_native_syscall_leave: swapcontext");
        }
    }
}

ssize_t _native_read(int fd, void *buf, size_t count)
{
    ssize_t r;

    _native_syscall_enter();
    r = real_read(fd, buf, count);
    _native_syscall_leave();

    return r;
}

ssize_t _native_write(int fd, const void *buf, size_t count)
{
    ssize_t r;

    _native_syscall_enter();
    r = real_write(fd, buf, count);
    _native_syscall_leave();

    return r;
}

ssize_t _native_writev(int fd, const struct iovec *iov, int iovcnt)
{
    ssize_t r;

    _native_syscall_enter();
    r = real_writev(fd, iov, iovcnt);
    _native_syscall_leave();

    return r;
}

int putchar(int c)
{
    _native_write(STDOUT_FILENO, &c, 1);
    return 0;
}

int puts(const char *s)
{
    int r;
    r = _native_write(STDOUT_FILENO, (char*)s, strlen(s));
    putchar('\n');
    return r;
}

/* Solve 'format string is not a string literal' as it is validly used in this
 * function */
__attribute__((__format__ (__printf__, 1, 0)))
char *make_message(const char *format, va_list argp)
{
    int size = 100;
    char *message, *temp;

    if ((message = malloc(size)) == NULL) {
        return NULL;
    }

    while (1) {
        int n = vsnprintf(message, size, format, argp);
        if (n < 0)
            return NULL;
        if (n < size)
            return message;
        size = n + 1;
        if ((temp = realloc(message, size)) == NULL) {
            free(message);
            return NULL;
        }
        else {
            message = temp;
        }
    }
}

int printf(const char *format, ...)
{
    int r;
    va_list argp;

    va_start(argp, format);
    r = vfprintf(stdout, format, argp);
    va_end(argp);

    return r;
}


int vprintf(const char *format, va_list argp)
{
    return vfprintf(stdout, format, argp);
}

int fprintf(FILE *fp, const char *format, ...)
{
    int r;
    va_list argp;

    va_start(argp, format);
    r = vfprintf(fp, format, argp);
    va_end(argp);

    return r;
}

int vfprintf(FILE *fp, const char *format, va_list argp)
{
    int r;
    char *m;

    if ((m = make_message(format, argp)) == NULL) {
        err(EXIT_FAILURE, "malloc");
    }
    r = _native_write(fileno(fp), m, strlen(m));
    free(m);

    return r;
}


void vwarn(const char *fmt, va_list args)
{
    char *m, *e;

    e = strerror(errno);

    if ((m = make_message(fmt, args)) == NULL) {
        _native_write(STDERR_FILENO, "malloc\n", 7);
        real_exit(EXIT_FAILURE);
    }
    _native_write(STDERR_FILENO, _progname, strlen(_progname));
    _native_write(STDERR_FILENO, ": ", 2);
    _native_write(STDERR_FILENO, m, strlen(m));
    _native_write(STDERR_FILENO, ": ", 2);
    _native_write(STDERR_FILENO, e, strlen(e));
    _native_write(STDERR_FILENO, "\n", 1);
    free(m);
}

void vwarnx(const char *fmt, va_list args)
{
    char *m;

    if ((m = make_message(fmt, args)) == NULL) {
        _native_write(STDERR_FILENO, "malloc\n", 7);
        real_exit(EXIT_FAILURE);
    }
    _native_write(STDERR_FILENO, _progname, strlen(_progname));
    _native_write(STDERR_FILENO, ": ", 2);
    _native_write(STDERR_FILENO, m, strlen(m));
    _native_write(STDERR_FILENO, "\n", 1);
    free(m);
}

void verr(int eval, const char *fmt, va_list args)
{
    vwarn(fmt, args);
    real_exit(eval);
}

void verrx(int eval, const char *fmt, va_list args)
{
    vwarnx(fmt, args);
    real_exit(eval);
}

void warn(const char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);
    vwarn(fmt, argp);
    va_end(argp);
}

void warnx(const char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);
    vwarnx(fmt, argp);
    va_end(argp);
}

void err(int eval, const char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);
    verr(eval, fmt, argp);
    va_end(argp);
}

void errx(int eval, const char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);
    verrx(eval, fmt, argp);
    va_end(argp);
}

int getpid(void)
{
    warnx("not implemented");
    return -1;
}

int _gettimeofday(struct timeval *tp, void *restrict tzp)
{
    (void) tzp;
    uint64_t now = xtimer_now_usec64(_native_instance);
    tp->tv_sec  = now / US_PER_SEC;
    tp->tv_usec = now - tp->tv_sec;
    return 0;
}

/**
 * set up native internal syscall symbols
 */
void _native_init_syscalls(void)
{
    *(void **)(&real_read) = dlsym(RTLD_NEXT, "read");
    *(void **)(&real_write) = dlsym(RTLD_NEXT, "write");
    *(void **)(&real_malloc) = dlsym(RTLD_NEXT, "malloc");
    *(void **)(&real_calloc) = dlsym(RTLD_NEXT, "calloc");
    *(void **)(&real_realloc) = dlsym(RTLD_NEXT, "realloc");
    *(void **)(&real_exit) = dlsym(RTLD_NEXT, "exit");
    *(void **)(&real_free) = dlsym(RTLD_NEXT, "free");
    *(void **)(&real_freeaddrinfo) = dlsym(RTLD_NEXT, "freeaddrinfo");
    *(void **)(&real_freeifaddrs) = dlsym(RTLD_NEXT, "freeifaddrs");
    *(void **)(&real_srandom) = dlsym(RTLD_NEXT, "srandom");
    *(void **)(&real_accept) = dlsym(RTLD_NEXT, "accept");
    *(void **)(&real_bind) = dlsym(RTLD_NEXT, "bind");
    *(void **)(&real_connect) = dlsym(RTLD_NEXT, "connect");
    *(void **)(&real_printf) = dlsym(RTLD_NEXT, "printf");
    *(void **)(&real_gai_strerror) = dlsym(RTLD_NEXT, "gai_strerror");
    *(void **)(&real_getaddrinfo) = dlsym(RTLD_NEXT, "getaddrinfo");
    *(void **)(&real_getifaddrs) = dlsym(RTLD_NEXT, "getifaddrs");
    *(void **)(&real_getpid) = dlsym(RTLD_NEXT, "getpid");
    *(void **)(&real_gettimeofday) = dlsym(RTLD_NEXT, "gettimeofday");
    *(void **)(&real_pipe) = dlsym(RTLD_NEXT, "pipe");
    *(void **)(&real_chdir) = dlsym(RTLD_NEXT, "chdir");
    *(void **)(&real_close) = dlsym(RTLD_NEXT, "close");
    *(void **)(&real_fcntl) = dlsym(RTLD_NEXT, "fcntl");
    *(void **)(&real_creat) = dlsym(RTLD_NEXT, "creat");
    *(void **)(&real_fork) = dlsym(RTLD_NEXT, "fork");
    *(void **)(&real_dup2) = dlsym(RTLD_NEXT, "dup2");
    *(void **)(&real_select) = dlsym(RTLD_NEXT, "select");
    *(void **)(&real_poll) = dlsym(RTLD_NEXT, "poll");
    *(void **)(&real_setitimer) = dlsym(RTLD_NEXT, "setitimer");
    *(void **)(&real_setsid) = dlsym(RTLD_NEXT, "setsid");
    *(void **)(&real_setsockopt) = dlsym(RTLD_NEXT, "setsockopt");
    *(void **)(&real_socket) = dlsym(RTLD_NEXT, "socket");
    *(void **)(&real_unlink) = dlsym(RTLD_NEXT, "unlink");
    *(void **)(&real_random) = dlsym(RTLD_NEXT, "random");
    *(void **)(&real_execve) = dlsym(RTLD_NEXT, "execve");
    *(void **)(&real_ioctl) = dlsym(RTLD_NEXT, "ioctl");
    *(void **)(&real_listen) = dlsym(RTLD_NEXT, "listen");
    *(void **)(&real_open) = dlsym(RTLD_NEXT, "open");
    *(void **)(&real_pause) = dlsym(RTLD_NEXT, "pause");
    *(void **)(&real_fopen) = dlsym(RTLD_NEXT, "fopen");
    *(void **)(&real_fread) = dlsym(RTLD_NEXT, "fread");
    *(void **)(&real_feof) = dlsym(RTLD_NEXT, "feof");
    *(void **)(&real_ferror) = dlsym(RTLD_NEXT, "ferror");
    *(void **)(&real_clearerr) = dlsym(RTLD_NEXT, "clearerr");
    *(void **)(&real_umask) = dlsym(RTLD_NEXT, "umask");
    *(void **)(&real_writev) = dlsym(RTLD_NEXT, "writev");
    *(void **)(&real_fclose) = dlsym(RTLD_NEXT, "fclose");
    *(void **)(&real_fseek) = dlsym(RTLD_NEXT, "fseek");
    *(void **)(&real_fputc) = dlsym(RTLD_NEXT, "fputc");
    *(void **)(&real_fgetc) = dlsym(RTLD_NEXT, "fgetc");
    *(void **)(&real_clock_gettime) = dlsym(RTLD_NEXT, "clock_gettime");
}
