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

#ifndef ARDUINO_BASE_HPP
#define ARDUINO_BASE_HPP

#include "arduino_board.h"

#include <arduino/serialport.hpp>

enum
{
    INPUT,       /* configure pin as input */
    OUTPUT,      /* configure pin as output */
    INPUT_PULLUP /* configure pin as input with pull-up resistor */
};

enum
{
    LOW  = 0, /* pin is cleared */
    HIGH = 1  /* pin is set */
};

#ifndef ARDUINO_UART_DEV
#define ARDUINO_UART_DEV VCDRIVERS_CONFIG_STDIOBASE_UART_DEV
#endif

static SerialPort Serial(ARDUINO_UART_DEV);

void pinMode(int pin, int mode);

void digitalWrite(int pin, int state);

int digitalRead(int pin);

void delay(unsigned long msec);

void delayMicroseconds(unsigned long usec);

unsigned long micros();

int analogRead(int pin);

#endif /* ARDUINO_BASE_HPP */
