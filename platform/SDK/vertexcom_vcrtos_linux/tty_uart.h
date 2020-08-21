#ifndef TTY_UART_H
#define TTY_UART_H

#include <vcdrivers/periph/uart.h>

#ifdef __cplusplus
extern "C" {
#endif

void tty_uart_setup(vcuart_t uart, const char *name);

void tty_uart_close(vcuart_t uart);

#ifdef __cplusplus
}
#endif

#endif /* TTY_UART_H */
