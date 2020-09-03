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

#define CLOCK_CONF_SECOND ZTIMER_CLOCK_SECOND
#define RTIMER_ARCH_SECOND ZTIMER_CLOCK_SECOND

#define INT_MASTER_CONF_STATUS_DATATYPE unsigned

typedef ztimer_now_t clock_time_t;
typedef unsigned int uip_stats_t;

// CSMA config
#define CSMA_CONF_SEND_SOFT_ACK 1

// ack wait timeout implement in radio driver
#define CSMA_CONF_ACK_WAIT_TIME 0
#define CSMA_CONF_AFTER_ACK_DETECTED_WAIT_TIME 0

// TCP config
#define UIP_CONF_TCP 1

// LOG config
#define LOG_CONF_LEVEL_MAC    LOG_LEVEL_DBG
#define LOG_CONF_LEVEL_FRAMER LOG_LEVEL_DBG
#define LOG_CONF_LEVEL_RPL    LOG_LEVEL_DBG

#ifdef __cplusplus
}
#endif

#endif /* CONTIKI_CONF_H */
