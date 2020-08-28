#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "native_internal.h"

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

void vcstdio_uart_rx_cb_handler(void *arg, uint8_t data)
{
    UartIsrpipe *isrpipe = static_cast<UartIsrpipe *>(arg);
    isrpipe->write_one(static_cast<char>(data));
}

void vcstdio_init(void *arg)
{
    Instance &instance = *static_cast<Instance *>(arg);

    uart_isrpipe = new (&uart_isrpipe_raw) UartIsrpipe(instance);

    vcuart_init(VCDRIVERS_CONFIG_STDIOBASE_UART_DEV, 115200,
                &vcstdio_uart_rx_cb_handler,
                static_cast<void *>(uart_isrpipe));
}

ssize_t vcstdio_write(const void *buffer, size_t len)
{
    return real_write(STDOUT_FILENO, buffer, len);
}

ssize_t vcstdio_read(void *buffer, size_t count)
{
    return uart_isrpipe->read(static_cast<char *>(buffer), count);
}

int vcstdio_read_available(void)
{
    return uart_isrpipe->get_tsrb().avail();
}