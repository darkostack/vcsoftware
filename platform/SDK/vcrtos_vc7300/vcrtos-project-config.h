/*
 * Copyright (c) 2020, Vertexcom Technologies, Inc.
 * All rights reserved.
 *
 * NOTICE: All information contained herein is, and remains
 * the property of Vertexcom Technologies, Inc. and its suppliers,
 * if any. The intellectual and technical concepts contained
 * herein are proprietary to Vertexcom Technologies, Inc.
 * and may be covered by U.S. and Foreign Patents, patents in process,
 * and protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Vertexcom Technologies, Inc.
 *
 * Authors: Darko Pancev <darko.pancev@vertexcom.com>
 */

#ifndef VCRTOS_PROJECT_CONFIG_H
#define VCRTOS_PROJECT_CONFIG_H

#define VCRTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE 0

#define VCRTOS_CONFIG_THREAD_FLAGS_ENABLE 1
#define VCRTOS_CONFIG_THREAD_EVENT_ENABLE 1

#define VCRTOS_CONFIG_ZTIMER_ENABLE 1
#define VCRTOS_CONFIG_ZTIMER_NOW64 0
#define VCRTOS_CONFIG_ZTIMER_USEC_BASE_FREQ (1000000LU)
#define VCRTOS_CONFIG_ZTIMER_USEC_DEV 0
#define VCRTOS_CONFIG_ZTIMER_USEC_MIN (10)
#define VCRTOS_CONFIG_ZTIMER_USEC_WIDTH (16)
#define VCRTOS_CONFIG_ZTIMER_EXTEND 1
#define VCRTOS_CONFIG_ZTIMER_CLOCK_SECOND (1200000LU)

#endif /* VCRTOS_PROJECT_CONFIG_H */
