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

#ifndef ARDUINO_BOARD_H
#define ARDUINO_BOARD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <vcdrivers/config.h>

#include <vcdrivers/periph/gpio.h>
#include <vcdrivers/periph/uart.h>

#include <vcdrivers/stdiobase.h>

#include "arduino_pinmap.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Look-up table for the Arduino's digital pins.
 */
static const vcgpio_t arduino_pin_map[] = {GPIO_PIN(PORTC, 1), GPIO_PIN(PORTC, 2)};

/**
 * Look-up table for the Arduino's analog pins.
 */
static const vcgpio_t arduino_analog_map[] = {GPIO_PIN(PORTA, 0), GPIO_PIN(PORTA, 1)};


#ifdef __cplusplus
}
#endif

#endif /* ARDUINO_BOARD_H */
