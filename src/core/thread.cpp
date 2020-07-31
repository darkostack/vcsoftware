#include <assert.h>

#include "core/instance.hpp"
#include "core/thread.hpp"
#include "core/code_utils.hpp"

namespace vc {

Thread *Thread::init(Instance &instances, char *stack, int stack_size, unsigned priority, int flags,
                     thread_handler_func_t handler_func, void *arg, const char *name)
{
    if (priority >= VCOS_CONFIG_THREAD_PRIORITY_LEVELS) return NULL;

    int total_stack_size = stack_size;

    /* aligned the stack on 16/32 bit boundary */
    uintptr_t misalignment = reinterpret_cast<uintptr_t>(stack) % 8;

    if (misalignment)
    {
        misalignment = 8 - misalignment;
        stack += misalignment;
        stack_size -= misalignment;
    }

    /* make room for the Thread */
    stack_size -= sizeof(Thread);

    /* round down the stacksize to multiple of Thread aligments (usually 16/32 bit) */
    stack_size -= stack_size % 8;

    if (stack_size < 0)
    {
        // TODO: warning: stack size is to small
    }

    /* allocate thread control block (tcb) at the top of our stackspace */
    Thread *tcb = (Thread *)(stack + stack_size);

    if (flags & THREAD_FLAGS_CREATE_STACKMARKER)
    {
        /* assign each int of the stack the value of it's address, for test purposes */
        uintptr_t *stackmax = reinterpret_cast<uintptr_t *>(stack + stack_size);
        uintptr_t *stackp = reinterpret_cast<uintptr_t *>(stack);

        while (stackp < stackmax)
        {
            *stackp = reinterpret_cast<uintptr_t>(stackp);
            stackp++;
        }
    }
    else
    {
        /* create stack guard */
        *(uintptr_t *)stack = reinterpret_cast<uintptr_t>(stack);
    }

    unsigned state = cpu_irq_disable();

    /* initialize instances for this thread */
#if VCOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
    tcb->instance = static_cast<void *>(&instances);
#else
    (void)instances;
#endif

    kernel_pid_t pid = KERNEL_PID_UNDEF;

    for (kernel_pid_t i = KERNEL_PID_FIRST; i <= KERNEL_PID_LAST; ++i)
    {
        if (tcb->get<ThreadScheduler>().get_thread_from_scheduler(i) == NULL)
        {
            pid = i;
            break;
        }
    }

    if (pid == KERNEL_PID_UNDEF)
    {
        cpu_irq_restore(state);
        return NULL;
    }

    tcb->get<ThreadScheduler>().set_thread_to_scheduler(tcb, pid);

    tcb->set_pid(pid);

    tcb->stack_init(handler_func, arg, stack, stack_size);

    tcb->set_stack_start(stack);

    tcb->set_stack_size(total_stack_size);

    tcb->set_name(name);

    tcb->set_priority(priority);

    tcb->set_status(THREAD_STATUS_STOPPED);

    tcb->init_runqueue_entry();

    tcb->init_msg();

    tcb->get<ThreadScheduler>().increment_numof_threads_in_scheduler();

    if (flags & THREAD_FLAGS_CREATE_SLEEPING)
    {
        tcb->get<ThreadScheduler>().set_thread_status(tcb, THREAD_STATUS_SLEEPING);
    }
    else
    {
        tcb->get<ThreadScheduler>().set_thread_status(tcb, THREAD_STATUS_PENDING);

        if (!(flags & THREAD_FLAGS_CREATE_WOUT_YIELD))
        {
            cpu_irq_restore(state);

            tcb->get<ThreadScheduler>().context_switch(priority);

            return tcb;
        }
    }

    cpu_irq_restore(state);

    return tcb;
}

void Thread::stack_init(thread_handler_func_t func, void *arg, void *stack_start, int stack_size)
{
    set_stack_pointer(thread_arch_stack_init(func, arg, stack_start, stack_size));
}

void Thread::add_to_list(List *list)
{
    assert(get_status() < THREAD_STATUS_RUNNING);

    uint8_t my_priority = get_priority();

    List *my_node = static_cast<List *>(get_runqueue_entry());

    while (list->next)
    {
        Thread *thread_on_list = Thread::get_thread_pointer_from_list_member(static_cast<List *>(list->next));

        if (thread_on_list->get_priority() > my_priority)
        {
            break;
        }

        list = static_cast<List *>(list->next);
    }

    my_node->next = list->next;

    list->next = my_node;
}

Thread *Thread::get_thread_pointer_from_list_member(List *list)
{
    list_node_t *node = static_cast<list_node_t *>(list);
    thread_t *thread = CONTAINER_OF(node, thread_t, runqueue_entry);
    return static_cast<Thread *>(thread);
}

void Thread::init_msg_queue(Msg *msg, int num)
{
    msg_array = msg;
    (static_cast<Cib *>(&msg_queue))->init(num);
}

int Thread::has_msg_queue(void)
{
    return msg_array != NULL;
}

int Thread::get_numof_msg_in_queue(void)
{
    int queued_msg = -1;

    if (has_msg_queue())
    {
        queued_msg = (static_cast<Cib *>(&msg_queue))->avail();
    }

    return queued_msg;
}

int Thread::is_pid_valid(kernel_pid_t pid)
{
    return ((KERNEL_PID_FIRST <= pid) && (pid <= KERNEL_PID_LAST));
}

int Thread::queued_msg(Msg *msg)
{
    int index = (static_cast<Cib *>(&msg_queue))->put();

    if (index < 0)
    {
        return 0;
    }

    Msg *dest = static_cast<Msg *>(&msg_array[index]);

    *dest = *msg;

    return 1;
}

void Thread::init_msg(void)
{
    wait_data = NULL;
    msg_waiters.next = NULL;
    (static_cast<Cib *>(&msg_queue))->init(0);
    msg_array = NULL;
}

void ThreadScheduler::run(void)
{
    disable_context_switch_request();

    Thread *current_thread = get_current_active_thread();

    Thread *next_thread = get_next_thread_from_runqueue();

    if (current_thread == next_thread) return;

    if (current_thread != NULL)
    {
        if (current_thread->get_status() == THREAD_STATUS_RUNNING)
        {
            current_thread->set_status(THREAD_STATUS_PENDING);
        }
    }

    next_thread->set_status(THREAD_STATUS_RUNNING);

    set_current_active_thread(next_thread);

    set_current_active_pid(next_thread->get_pid());
}

void ThreadScheduler::set_thread_status(Thread *thread, thread_status_t status)
{
    uint8_t priority = thread->get_priority();

    if (status >= THREAD_STATUS_RUNNING)
    {
        if (thread->get_status() < THREAD_STATUS_RUNNING)
        {
            list_node_t *thread_runqueue_entry = thread->get_runqueue_entry();

            scheduler_runqueue[priority].right_push(static_cast<Clist *>(thread_runqueue_entry));

            set_runqueue_bitcache(priority);
        }
    }
    else
    {
        if (thread->get_status() >= THREAD_STATUS_RUNNING)
        {
            scheduler_runqueue[priority].left_pop();

            if (scheduler_runqueue[priority].next == NULL)
            {
                reset_runqueue_bitcache(priority);
            }
        }
    }

    thread->set_status(status);
}

void ThreadScheduler::context_switch(uint8_t priority_to_switch)
{
    Thread *current_thread = get_current_active_thread();

    uint8_t current_priority = current_thread->get_priority();

    int is_in_runqueue = (current_thread->get_status() >= THREAD_STATUS_RUNNING);

    /* Note: the lowest priority number is the highest priority thread */

    if (!is_in_runqueue || (current_priority > priority_to_switch))
    {
        if (cpu_is_in_isr())
        {
            enable_context_switch_request();
        }
        else
        {
            yield_higher_priority_thread();
        }
    }
}

void ThreadScheduler::yield_higher_priority_thread(void)
{
    cpu_trigger_pendsv_interrupt();
}

uint8_t ThreadScheduler::get_lsb_index_from_runqueue(void)
{
    uint8_t index = 0;

    uint32_t queue = get_runqueue_bitcache();

    /* [IMPORTANT]: this functions assume there will be at least 1 thread on the queue, (idle) thread */

    while ((queue & 0x01) == 0)
    {
        queue >>= 1;
        index += 1;
    }

    return index;
}

Thread *ThreadScheduler::get_next_thread_from_runqueue(void)
{
    uint8_t thread_idx = get_lsb_index_from_runqueue();

    list_node_t *thread_ptr_in_queue = static_cast<list_node_t *>((scheduler_runqueue[thread_idx].next)->next);

    thread_t *thread = CONTAINER_OF(thread_ptr_in_queue, thread_t, runqueue_entry);

    return static_cast<Thread *>(thread);
}

void ThreadScheduler::sleeping_current_thread(void)
{
    if (cpu_is_in_isr())
    {
        return;
    }

    unsigned state = cpu_irq_disable();

    set_thread_status(get_current_active_thread(), THREAD_STATUS_SLEEPING);

    cpu_irq_restore(state);

    yield_higher_priority_thread();
}

int ThreadScheduler::wakeup_thread(kernel_pid_t pid)
{
    unsigned state = cpu_irq_disable();

    Thread *thread_to_wakeup = get_thread_from_scheduler(pid);

    if (!thread_to_wakeup)
    {
        //TODO: Warning thread does not exist!
    }
    else if (thread_to_wakeup->get_status() == THREAD_STATUS_SLEEPING)
    {
        set_thread_status(thread_to_wakeup, THREAD_STATUS_RUNNING);

        cpu_irq_restore(state);

        context_switch(thread_to_wakeup->get_priority());

        return 1;
    }
    else
    {
        // TODO: Warning thread is not sleeping!
    }

    cpu_irq_restore(state);

    return (int)THREAD_STATUS_NOT_FOUND;
}

void ThreadScheduler::yield(void)
{
    unsigned state = cpu_irq_disable();

    Thread *current_thread = get_current_active_thread();

    if (current_thread->get_status() >= THREAD_STATUS_RUNNING)
    {
        scheduler_runqueue[current_thread->get_priority()].left_pop_right_push();
    }

    cpu_irq_restore(state);

    yield_higher_priority_thread();
}

const char *ThreadScheduler::thread_status_to_string(thread_status_t status)
{
    const char *retval;

    switch (status)
    {
    case THREAD_STATUS_RUNNING:
        retval = "running";
        break;

    case THREAD_STATUS_PENDING:
        retval = "pending";
        break;

    case THREAD_STATUS_STOPPED:
        retval = "stopped";
        break;

    case THREAD_STATUS_SLEEPING:
        retval = "sleeping";
        break;

    case THREAD_STATUS_MUTEX_BLOCKED:
        retval = "bl mutex";
        break;

    case THREAD_STATUS_RECEIVE_BLOCKED:
        retval = "bl rx";
        break;

    case THREAD_STATUS_SEND_BLOCKED:
        retval = "bl send";
        break;

    case THREAD_STATUS_REPLY_BLOCKED:
        retval = "bl reply";
        break;

    default:
        retval = "unknown";
        break;
    }

    return retval;
}

thread_flags_t ThreadFlags::clear_atomic(Thread *thread, thread_flags_t mask)
{
    unsigned state = cpu_irq_disable();

    mask &= thread->flags;

    thread->flags &= ~mask;

    cpu_irq_restore(state);

    return mask;
}

void ThreadFlags::wait(thread_flags_t mask, Thread *thread, thread_status_t thread_status, unsigned irqstate)
{
    thread->wait_data = static_cast<void *>(&mask);

    get<ThreadScheduler>().set_thread_status(thread, thread_status);

    cpu_irq_restore(irqstate);

    ThreadScheduler::yield_higher_priority_thread();
}

void ThreadFlags::wait_any_blocked(thread_flags_t mask)
{
    Thread *current_thread = get<ThreadScheduler>().get_current_active_thread();

    unsigned state = cpu_irq_disable();

    if (!(current_thread->flags & mask))
    {
        wait(mask, current_thread, THREAD_STATUS_FLAG_BLOCKED_ANY, state);
    }
    else
    {
        cpu_irq_restore(state);
    }
}

void ThreadFlags::set(Thread *thread, thread_flags_t mask)
{
    unsigned state = cpu_irq_disable();

    thread->flags |= mask;

    if (wake(thread))
    {
        cpu_irq_restore(state);

        ThreadScheduler::yield_higher_priority_thread();
    }
    else
    {
        cpu_irq_restore(state);
    }
}

int ThreadFlags::wake(Thread *thread)
{
    unsigned wakeup;

    thread_flags_t mask = *static_cast<uint16_t *>(thread->wait_data);

    switch (thread->get_status())
    {
    case THREAD_STATUS_FLAG_BLOCKED_ANY:
        wakeup = (thread->flags & mask);
        break;

    case THREAD_STATUS_FLAG_BLOCKED_ALL:
        wakeup = ((thread->flags & mask) == mask);
        break;

    default:
        wakeup = 0;
        break;
    }

    if (wakeup)
    {
        get<ThreadScheduler>().set_thread_status(thread, THREAD_STATUS_PENDING);
        get<ThreadScheduler>().enable_context_switch_request();
    }

    return wakeup;
}

thread_flags_t ThreadFlags::clear(thread_flags_t mask)
{
    Thread *current_thread = get<ThreadScheduler>().get_current_active_thread();
    mask = clear_atomic(current_thread, mask);
    return mask;
}

thread_flags_t ThreadFlags::wait_any(thread_flags_t mask)
{
    Thread *thread = get<ThreadScheduler>().get_current_active_thread();
    wait_any_blocked(mask);
    return clear_atomic(thread, mask);
}

thread_flags_t ThreadFlags::wait_all(thread_flags_t mask)
{
    unsigned state = cpu_irq_disable();

    Thread *current_thread = get<ThreadScheduler>().get_current_active_thread();

    if (!((current_thread->flags & mask) == mask))
    {
        wait(mask, current_thread, THREAD_STATUS_FLAG_BLOCKED_ALL, state);
    }
    else
    {
        cpu_irq_restore(state);
    }

    return clear_atomic(current_thread, mask);
}

thread_flags_t ThreadFlags::wait_one(thread_flags_t mask)
{
    wait_any_blocked(mask);

    Thread *current_thread = get<ThreadScheduler>().get_current_active_thread();

    thread_flags_t tmp = current_thread->flags & mask;

    /* clear all but least significant bit */
    tmp &= (~tmp + 1);

    return clear_atomic(current_thread, tmp);
}

extern "C" void cpu_end_of_isr(instance_t *instances)
{
    Instance &instance = *static_cast<Instance *>(instances);

    if (instance.get<ThreadScheduler>().is_context_switch_requested())
    {
        ThreadScheduler::yield_higher_priority_thread();
    }
}

template <> inline Instance &Thread::get(void) const
{
    return get_instance();
}

template <typename Type> inline Type &Thread::get(void) const
{
    return get_instance().get<Type>();
}

template <typename Type> inline Type &ThreadFlags::get(void) const
{
    return get_instance().get<Type>();
}

} // namespace vc
