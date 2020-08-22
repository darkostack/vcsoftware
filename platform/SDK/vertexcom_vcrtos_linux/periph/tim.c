#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

#include <vcrtos/cpu.h>
#include <vcdrivers/periph/tim.h>

#define TIMER_NUMOF 1
#define NATIVE_TIMER_MIN_RES 200

#define NATIVE_TIMER_SPEED 1000000

static unsigned long time_null;

static timer_cb_t _callback;
static void *_cb_arg;

static struct itimerval itv;

static unsigned long ts2ticks(struct timespec *tp)
{
    /* TODO: check for overflow */
    return(((unsigned long)tp->tv_sec * NATIVE_TIMER_SPEED) + (tp->tv_nsec / 1000));
}

void native_isr_timer(void)
{
    _callback(_cb_arg, 0);
}

int vctim_init(vctim_t dev, unsigned long freq, vctim_callback_func_t callback, void *arg)
{
    if (dev >= TIMER_NUMOF)
    {
        return -1;
    }

    if (freq != NATIVE_TIMER_SPEED)
    {
        return -1;
    }

    /* initialize time delta */
    time_null = 0;
    time_null = vctim_read(dev);

    _callback = cb;
    _cb_arg = arg;

    register_interrupt(SIGALRM, native_isr_timer);
}

static void do_timer_set(unsigned int offset)
{
    if (offset && offset < NATIVE_TIMER_MIN_RES)
    {
        offset = NATIVE_TIMER_MIN_RES;
    }

    memset(&itv, 0, sizeof(itv));

    itv.it_value.tv_sec = (offset / 1000000);
    itv.it_value.tv_usec = offset % 1000000;

    _native_syscall_enter();
    if (real_setitimer(ITIMER_REAL, &itv, NULL) == -1) {
        err(EXIT_FAILURE, "timer_arm: setitimer");
    }
    _native_syscall_leave();
}

int vctim_set(vctim_t dev, int channel, unsigned int offset)
{
    (void)dev;

    if (channel != 0) {
        return -1;
    }

    if (!offset) {
        offset = NATIVE_TIMER_MIN_RES;
    }

    do_timer_set(offset);

    return 0;
}

int vctim_set_absolute(vctim_t dev, int channel, unsigned int value)
{
    uint32_t now = vctim_read(dev);
    return vctim_set(dev, channel, value - now);
}

int vctim_clear(vctim_t dev, int channel)
{
    (void)dev;
    (void)channel;

    do_timer_set(0);

    return 0;
}

void vctim_start(vctim_t dev)
{
    (void)dev;
}

void vctim_stop(vctim_t dev)
{
    (void)dev;
}

unsigned int vctim_read(vctim_t dev)
{
    if (dev >= TIMER_NUMOF) {
        return 0;
    }

    struct timespec t;

    _native_syscall_enter();
    if (real_clock_gettime(CLOCK_MONOTONIC, &t) == -1) {
        err(EXIT_FAILURE, "timer_read: clock_gettime");
    }
    _native_syscall_leave();

    return ts2ticks(&t) - time_null;
}
