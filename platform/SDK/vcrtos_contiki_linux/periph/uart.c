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

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>

#include <vcrtos/cpu.h>
#include <vcdrivers/periph/uart.h>

#include "native_internal.h"
#include "tty_uart.h"
#include "async_read.h"

#define UART_NUMOF 1

static vcuart_isr_context_t uart_config[UART_NUMOF];

static char *tty_device_filenames[UART_NUMOF];

static int tty_fds[UART_NUMOF];

void tty_uart_setup(vcuart_t uart, const char *filename)
{
    tty_device_filenames[uart] = strndup(filename, PATH_MAX - 1);
}

void tty_uart_close(vcuart_t uart)
{
    real_close(tty_fds[uart]);
}

static void io_signal_handler(int fd, void *arg)
{
    vcuart_t uart;
    (void) arg;

    for (uart = 0; uart < UART_NUMOF; uart++)
    {
        if (tty_fds[uart] == fd)
        {
            break;
        }
    }

    while (1)
    {
        char c;
        int status = real_read(fd, &c, 1);

        if (status == 1)
        {
            uart_config[uart].callback(uart_config[uart].arg, c);
            cpu_end_of_isr();
        }
        else
        {
            if (status == -1 && errno != EAGAIN)
            {
                uart_config[uart].callback = NULL;
            }
            break;
        }
    }

    native_async_read_continue(fd);
}

int vcuart_init(vcuart_t uart, uint32_t baudrate, vcuart_rx_callback_func_t callback, void *arg)
{
    if (uart >= UART_NUMOF)
    {
        return -1;
    }

    struct termios termios;

    memset(&termios, 0, sizeof(termios));

    termios.c_iflag = 0;
    termios.c_oflag = ONLCR; /* map NL to CR-NL on output */
    termios.c_cflag = CS8 | CREAD | CLOCAL;
    termios.c_lflag = 0;

    speed_t speed;

    switch (baudrate)
    {
    case 9600: speed = B9600; break;
    case 19200: speed = B19200; break;
    case 38400: speed = B38400; break;
    case 57600: speed = B57600; break;
    case 115200: speed = B115200; break;
    default:
        return -1;
        break;
    }

    cfsetospeed(&termios, speed);
    cfsetispeed(&termios, speed);

    tty_fds[uart] = real_open(tty_device_filenames[uart], O_RDWR | O_NONBLOCK);

    if (tty_fds[uart] < 0)
    {
        return -1;
    }

    tcsetattr(tty_fds[uart], TCSANOW, &termios);

    uart_config[uart].callback = callback;
    uart_config[uart].arg = arg;

    native_async_read_setup();
    native_async_read_add_handler(tty_fds[uart], NULL, io_signal_handler);

    return 0;
}

size_t vcuart_write(vcuart_t uart, const uint8_t *data, size_t len)
{
    _native_write(tty_fds[uart], data, len);
    return len;
}

void vcuart_power_on(vcuart_t uart)
{
    (void) uart;
}

void vcuart_power_off(vcuart_t uart)
{
    (void) uart;
}
