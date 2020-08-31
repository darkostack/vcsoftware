#ifndef ASYNC_READ_H
#define ASYNC_READ_H

#include <stdlib.h>
#include <poll.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ASYNC_READ_NUMOF
#define ASYNC_READ_NUMOF 2
#endif

typedef void (*native_async_read_callback_t)(int fd, void *arg);

typedef struct 
{
    pid_t child_pid;
    native_async_read_callback_t cb;
    void *arg;
    struct pollfd *fd;
} async_read_t;

void native_async_read_setup(void);

void native_async_read_cleanup(void);

void native_async_read_continue(int fd);

void native_async_read_add_handler(int fd, void *arg, native_async_read_callback_t handler);

void native_async_read_add_int_handler(int fd, void *arg, native_async_read_callback_t handler);

#ifdef __cplusplus
}
#endif

#endif /* ASYNC_READ_H */
