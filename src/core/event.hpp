#ifndef CORE_EVENT_HPP
#define CORE_EVENT_HPP

#include <string.h>
#include <assert.h>

#include <vcos/config.h>
#include <vcos/event.h>

namespace vc {

class Instance;
class Clist;
class Thread;

class Event : public event_t
{
public:
    explicit Event(event_handler_func_t func)
    {
        handler = func;
    }
};

class EventQueue
{
public:
    explicit EventQueue(Instance *instances)
    {
        instance = static_cast<void *>(instances);
    }

    void init_detached(void) { memset(this, '\0', sizeof(*this)); }

    void init(void);

    void init(char *stack, size_t stack_size, unsigned priority);

    void claim(void);

    void post(Event *event);

    void cancel(Event *event);

    Event *get(void);

    Event *wait(void);

    void loop(void);

private:
    template <typename Type> inline Type &get(void) const;

    Instance &get_instance(void) const { return *static_cast<Instance *>(instance); }

    Clist event_list;

    Thread *waiter;

    void *instance;
};

} // namespace vc

#endif /* CORE_EVENT_HPP */
