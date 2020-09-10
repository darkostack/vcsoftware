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

#include <arduino/base.hpp>

void pinMode(int pin, int mode)
{
    vcgpio_mode_t m = GPIO_OUT;

    if (mode == INPUT)
    {
        m = GPIO_IN;
    }
    else if (mode == INPUT_PULLUP)
    {
        m = GPIO_IN_PU;
    }

    vcgpio_init(arduino_pin_map[pin], m);
}

void digitalWrite(int pin, int state)
{
    vcgpio_write(arduino_pin_map[pin], state);
}

int digitalRead(int pin)
{
    if (vcgpio_read(arduino_pin_map[pin]))
    {
        return HIGH;
    }
    else
    {
        return LOW;
    }
}

void delay(unsigned long msec)
{
    (void) msec;
}

void delayMicroseconds(unsigned long usec)
{
    (void) usec;
}

unsigned long micros()
{
    return 0;
}

int analogRead(int pin)
{
    (void) pin;
    return 0;
}
