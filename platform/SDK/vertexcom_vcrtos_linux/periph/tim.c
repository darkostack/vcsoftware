#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

#include <vcrtos/cpu.h>
#include <vcdrivers/periph/tim.h>

#define NATIVE_TIMER_SPEED 1000000

static unsigned long time_null;

static timer_cb_t _callback;
static void *_cb_arg;

static struct itimerval itv;

static unsigned long ts2ticks(struct timespec *tp)
{
}
