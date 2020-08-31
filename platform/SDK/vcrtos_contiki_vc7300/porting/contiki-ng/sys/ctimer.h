#ifndef CTIMER_H
#define CTIMER_H

#include "process.h"

#include <vcrtos/config.h>
#include <vcrtos/ztimer.h>

#include "etimer.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ctimer
{
    ztimer_t super;
    struct timer timer;
    struct process *p;
    void (*cb)(void *);
    void *arg;
};

void ctimer_init(void);

void ctimer_set(struct ctimer *ct, clock_time_t interval, void (*cb)(void *), void *arg);

void ctimer_reset(struct ctimer *ct);

void ctimer_set_with_process(struct ctimer *ct, clock_time_t interval,
                             void (*cb)(void *), void *arg, struct process *p);

void ctimer_stop(struct ctimer *ct);

int ctimer_expired(struct ctimer *ct);

#ifdef __cplusplus
}
#endif

#endif /* CTIMER_H */
