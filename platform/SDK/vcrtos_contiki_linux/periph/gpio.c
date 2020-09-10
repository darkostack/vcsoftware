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

#include <vcdrivers/periph/gpio.h>

int vcgpio_init(vcgpio_t pin, vcgpio_mode_t mode)
{
    (void) pin;
    (void) mode;
    return (mode >= GPIO_OUT) ? 0 : -1;
}

int vcgpio_read(vcgpio_t pin)
{
    (void) pin;
    return 0;
}

void vcgpio_set(vcgpio_t pin)
{
    (void) pin;
}

void vcgpio_clear(vcgpio_t pin)
{
    (void) pin;
}

void vcgpio_toggle(vcgpio_t pin)
{
    (void) pin;
}

void vcgpio_write(vcgpio_t pin, int value)
{
    (void) pin;
    (void) value;
}
