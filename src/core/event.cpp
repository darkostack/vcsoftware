#include "core/instance.hpp"
#include "core/thread.hpp"
#include "core/event.hpp"

namespace vc {

void EventQueue::init(void)
{
    memset(this, '\0', sizeof(*this));

    waiter = get<ThreadScheduler>().get_current_active_thread();
}

void EventQueue::claim(void)
{
    assert(waiter == NULL);

    waiter = get<ThreadScheduler>().get_current_active_thread();
}

void EventQueue::post(Event *event)
{
    assert(event);

    unsigned state = cpu_irq_disable();

    if (!event->list_node.next)
    {
        event_list.right_push(static_cast<Clist *>(&event->list_node));
    }

    Thread *thread_waiter = this->waiter;

    cpu_irq_restore(state);

    if (thread_waiter != NULL)
    {
        get<ThreadFlags>().set(thread_waiter, THREAD_FLAG_EVENT);
    }
}

void EventQueue::cancel(Event *event)
{
    assert(event);

    unsigned state = cpu_irq_disable();

    event_list.remove(static_cast<Clist *>(&event->list_node));

    event->list_node.next = NULL;

    cpu_irq_restore(state);
}

Event *EventQueue::get(void)
{
    unsigned state = cpu_irq_disable();

    Event *result = reinterpret_cast<Event *>(event_list.left_pop());

    cpu_irq_restore(state);

    if (result)
    {
        result->list_node.next = NULL;
    }

    return result;
}

Event  *EventQueue::wait(void)
{
    Event *result;

#ifdef UNITTEST

    unsigned state = cpu_irq_disable();
    result = reinterpret_cast<Event *>(event_list.left_pop());
    cpu_irq_restore(state);

    if (result == NULL)
    {
        get<ThreadFlags>().wait_any(THREAD_FLAG_EVENT);
    }

#else

    do
    {
        unsigned state = cpu_irq_disable();
        result = reinterpret_cast<Event *>(event_list.left_pop());
        cpu_irq_restore(state);

        if (result == NULL)
        {
            get<ThreadFlags>().wait_any(THREAD_FLAG_EVENT);
        }

    } while (result == NULL);

#endif

    result->list_node.next = NULL;

    return result;
}

void EventQueue::loop(void)
{
    Event *event;

#ifdef UNITTEST

    event = wait();

    if (event)
    {
        event->handler(static_cast<event_t *>(event));
    }

#else

    while ((event = wait()))
    {
        event->handler(static_cast<event_t *>(event));
    }

#endif
}

extern "C" void *_event_handler(void *event_queue)
{
    EventQueue *queue = static_cast<EventQueue *>(event_queue);

    queue->claim();

    queue->loop();

    /* should not reach here */

    return NULL;
}

void EventQueue::init(char *stack, size_t stack_size, unsigned priority)
{
    Thread::init(get_instance(), stack, stack_size, priority, 0, _event_handler, this, "event");
}

template <typename Type> inline Type &EventQueue::get(void) const
{
    return get_instance().get<Type>();
}

} // namespace vc
