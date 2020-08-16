#ifndef PROCESS_H
#define PROCESS_H

#include <vcrtos/config.h>
#include <vcrtos/instance.h>
#include <vcrtos/thread.h>
#include <vcrtos/mutex.h>
#include <vcrtos/event.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PROCESS_MAX_EVENTS
#define PROCESS_MAX_EVENTS KERNEL_MAXTHREADS
#endif

typedef unsigned char process_event_t; /* event id */
typedef void *process_data_t;

typedef enum
{
    PROCESS_EVENT_PRIO_HIGH,
    PROCESS_EVENT_PRIO_MEDIUM,
    PROCESS_EVENT_PRIO_LOW,
    PROCESS_EVENT_PRIO_NUMOF
} process_event_prio_t;

#define PROCESS_EVENT_NONE 0
#define PROCESS_EVENT_INIT 1
#define PROCESS_EVENT_POLL 2
#define PROCESS_EVENT_EXIT 3
#define PROCESS_EVENT_SERVICE_REMOVED 4
#define PROCESS_EVENT_CONTINUE 5
#define PROCESS_EVENT_MSG 6
#define PROCESS_EVENT_EXITED 7
#define PROCESS_EVENT_TIMER 8
#define PROCESS_EVENT_COM 9

#define PROCESS_EVENT_LAST 10

typedef struct
{
    event_t super;
    kernel_pid_t target;
    process_event_prio_t priority;
    unsigned char event_id;
    void *data;
} process_custom_event_t;

typedef struct
{
    mutex_t *mutex;
    process_event_t ev;
    process_data_t data;
} process_custom_data_t;

struct process
{
    kernel_pid_t pid;
    char *stack;
    size_t stack_size;
    const char *process_name;
    thread_handler_func_t thread_handler;
};

#define PROCESS(name, strname, size) \
    static char process_stack_##name[size]; \
    void *thread_handler_##name(void *arg); \
    struct process name = {  \
        .pid = KERNEL_PID_UNDEF, \
        .stack = process_stack_##name, \
        .stack_size = size, \
        .process_name = strname, \
        .thread_handler = thread_handler_##name \
    }; \
    static void process_func_##name(struct process *p, process_event_t ev, process_data_t data)

#define PROCESS_NAME(name) extern struct process name

#define PROCESS_THREAD(name, ev, data) \
    void *thread_handler_##name(void *arg) \
    { \
        name.pid = thread_current_pid(process_instance); \
        process_event_t start_event = PROCESS_EVENT_INIT; \
        process_data_t start_data = (process_data_t *)arg; \
        process_func_##name(&name, start_event, start_data); \
        return NULL; \
    } \
    static void process_func_##name(struct process *p, process_event_t ev, process_data_t data)

#define PROCESS_BEGIN() \
    _process_data[p->pid].ev = ev; \
    _process_data[p->pid].data = data; \
    process_current = p; \
    if (_process_data[p->pid].mutex->queue.next == NULL) mutex_lock(_process_data[p->pid].mutex)

#define PROCESS_END() \
    process_current = NULL; \
    thread_exit(process_instance)

#define PROCESS_WAIT_EVENT() \
    do { \
        ev = PROCESS_EVENT_NONE; \
        data = NULL; \
        while (_process_data[p->pid].ev == PROCESS_EVENT_NONE) \
        { \
            mutex_lock(_process_data[p->pid].mutex); \
        } \
        process_current = p; \
        ev = _process_data[p->pid].ev; \
        data = _process_data[p->pid].data; \
    } while (0)

#define PROCESS_WAIT_EVENT_UNTIL(cond) \
    do { \
        PROCESS_WAIT_EVENT(); \
        if (cond) \
        { \
            break; \
        } \
    } while (0)

#define PROCESS_CURRENT() process_current

#define PROCESS_YIELD() thread_yield(process_instance)

void process_init(void *instance);

void process_start(struct process *p, process_data_t data);

int process_post(struct process *p, process_event_t event, process_data_t data);

process_event_t process_alloc_event(process_event_prio_t prio);

extern process_custom_data_t _process_data[KERNEL_MAXTHREADS];
extern process_custom_event_t _process_events[KERNEL_MAXTHREADS];
extern void *process_instance;
extern struct process *process_current;

#ifdef __cplusplus
}
#endif

#endif /* PROCESS_H */
