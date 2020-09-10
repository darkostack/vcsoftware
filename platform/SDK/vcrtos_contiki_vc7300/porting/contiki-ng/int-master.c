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

#include "contiki.h"
#include "sys/int-master.h"

#include <vcdrivers/cpu.h>
#include <vcrtos/cpu.h>

void int_master_enable(void)
{
    cpu_irq_enable();
}

int_master_status_t int_master_read_and_disable(void)
{
    return (int_master_status_t)cpu_irq_disable();
}

void int_master_status_set(int_master_status_t status)
{
    cpu_irq_restore((unsigned)status);
}

bool int_master_is_enable(void)
{
    return __get_PRIMASK() ? 1 : 0;
}
