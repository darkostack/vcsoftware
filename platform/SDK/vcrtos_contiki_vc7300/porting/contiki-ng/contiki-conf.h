#ifndef CONTIKI_CONF_H
#define CONTIKI_CONF_H

#include <stdint.h>
#include <string.h>
#include <inttypes.h>

#include <vcrtos/config.h>
#include <vcrtos/ztimer.h>
#include <vcrtos/thread.h>
#include <vcrtos/instance.h>
#include <vcrtos/assert.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CLOCK_CONF_SECOND 1000000
#define RTIMER_ARCH_SECOND 1000000

#define INT_MASTER_CONF_STATUS_DATATYPE unsigned

typedef ztimer_now_t clock_time_t;
typedef unsigned int uip_stats_t;

#ifdef __cplusplus
}
#endif

#endif /* CONTIKI_CONF_H */
