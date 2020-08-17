#include <vcrtos/assert.h>
#include <vcrtos/cpu.h>

#include "process.h"

extern event_queue_t event_queue_highest;
extern event_queue_t event_queue_medium;
extern event_queue_t event_queue_lowest;

static process_event_t lastevent = PROCESS_EVENT_LAST;

mutex_t process_mutex[KERNEL_MAXTHREADS];

process_custom_data_t _process_data[KERNEL_MAXTHREADS];
process_custom_event_t _process_events[KERNEL_MAXTHREADS];

void *process_instance = NULL;
struct process *process_current = NULL;

void _process_event_default_handler(event_t *event)
{
    process_custom_event_t *custom_event = (process_custom_event_t *)event;

    unsigned state = cpu_irq_disable();

    _process_data[custom_event->target].ev = custom_event->event_id;
    _process_data[custom_event->target].data = custom_event->data;

    cpu_irq_restore(state);

    mutex_unlock(_process_data[custom_event->target].mutex);
}

void process_init(void *instance)
{
    process_instance = instance;

    unsigned i = 0;

    for (i = 0; i < KERNEL_MAXTHREADS; i++)
    {
        _process_data[i].mutex = &process_mutex[i];
        _process_data[i].ev = PROCESS_EVENT_NONE;
        _process_data[i].data = NULL;

        mutex_init(instance, _process_data[i].mutex);
    }

    for (i = 0; i < KERNEL_MAXTHREADS; i++)
    {
        _process_events[i].super.list_node.next = NULL;
        _process_events[i].super.handler = _process_event_default_handler;
        _process_events[i].target = KERNEL_PID_UNDEF;
        _process_events[i].priority = PROCESS_EVENT_PRIO_MEDIUM;
        _process_events[i].event_id = i;
        _process_events[i].data = NULL;
    }
}

void process_start(struct process *p, process_data_t data)
{
    vcassert(p != NULL && p->pid == KERNEL_PID_UNDEF && process_instance != NULL);

    p->pid = thread_create(process_instance, p->stack, p->stack_size, KERNEL_THREAD_PRIORITY_MAIN,
                           THREAD_FLAGS_CREATE_WOUT_YIELD | THREAD_FLAGS_CREATE_STACKMARKER,
                           p->thread_handler, (void *)data, p->process_name);
}

int process_post(struct process *p, process_event_t event, process_data_t data)
{
    vcassert(_process_events[event].event_id == event);

    process_event_prio_t prio = _process_events[event].priority;

    unsigned state = cpu_irq_disable();

    _process_events[event].target = p->pid;
    _process_events[event].data = data;

    cpu_irq_restore(state);

    switch (prio)
    {
    case PROCESS_EVENT_PRIO_HIGH:
        event_post(&event_queue_highest, (event_t *)&_process_events[event]);
        break;

    case PROCESS_EVENT_PRIO_MEDIUM:
        event_post(&event_queue_medium, (event_t *)&_process_events[event]);
        break;

    case PROCESS_EVENT_PRIO_LOW:
        event_post(&event_queue_lowest, (event_t *)&_process_events[event]);
        break;

    default:
        return -1;
    }

    return 0;
}

process_event_t process_alloc_event(process_event_prio_t prio)
{
    vcassert(lastevent < KERNEL_MAXTHREADS);
    unsigned char event_id = lastevent++;
    unsigned state = cpu_irq_disable();
    _process_events[event_id].priority = prio;
    cpu_irq_restore(state);
    return event_id;
}
