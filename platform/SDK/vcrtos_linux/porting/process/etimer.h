#ifndef ETIMER_H
#define ETIMER_H

#include "process.h"

#include <vcrtos/config.h>
#include <vcrtos/ztimer.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t clock_time_t;

struct etimer
{
    ztimer_t super;
    uint64_t start;
    uint32_t interval;
    struct process *p;
};

void etimer_set(struct etimer *et, clock_time_t interval);

void etimer_reset(struct etimer *et);

void etimer_reset_with_new_interval(struct etimer *et, clock_time_t interval);

void etimer_restart(struct etimer *et);

clock_time_t etimer_expiration_time(struct etimer *et);

clock_time_t etimer_start_time(struct etimer *et);

int etimer_expired(struct etimer *et);

void etimer_stop(struct etimer *et);

#ifdef __cplusplus
}
#endif

#endif /* ETIMER_H */
