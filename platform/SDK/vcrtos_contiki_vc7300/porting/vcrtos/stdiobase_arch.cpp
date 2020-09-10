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

#include <vcdrivers/periph/uart.h>
#include <vcdrivers/stdiobase.h>

#include "core/code_utils.h"
#include "core/instance.hpp"
#include "core/new.hpp"

#include "utils/isrpipe.hpp"

using namespace vc;
using namespace utils;

static DEFINE_ALIGNED_VAR(uart_isrpipe_raw, sizeof(UartIsrpipe), uint64_t);

UartIsrpipe *uart_isrpipe;

void vcstdio_uart_rx_callback_handler(void *arg, uint8_t data)
{
    UartIsrpipe *isrpipe = static_cast<UartIsrpipe *>(arg);
    isrpipe->write_one(static_cast<char>(data));
}

void vcstdio_init(void *instance)
{
    Instance &instances = *static_cast<Instance *>(instance);

    uart_isrpipe = new (&uart_isrpipe_raw) UartIsrpipe(instances);

    vcuart_init(VCDRIVERS_CONFIG_STDIOBASE_UART_DEV,
                115200,
                &vcstdio_uart_rx_callback_handler,
                static_cast<void *>(uart_isrpipe));
}

ssize_t vcstdio_read(void *buffer, size_t count)
{
    return uart_isrpipe->read(static_cast<char *>(buffer), count);
}

int vcstdio_read_available(void)
{
    return uart_isrpipe->get_tsrb().avail();
}
