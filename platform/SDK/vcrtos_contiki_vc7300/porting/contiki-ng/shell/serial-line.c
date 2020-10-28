#include <vcdrivers/stdiobase.h>

#include "dev/serial-line.h"

#include <stdio.h>
#include <string.h>

#ifdef SERIAL_LINE_CONF_BUFSIZE
#define BUFSIZE SERIAL_LINE_CONF_BUFSIZE
#else
#define BUFSIZE 128
#endif

#if (BUFSIZE & (BUFSIZE - 1)) != 0
#error SERIAL_LINE_CONF_BUFSIZE must be a power of two (i.e., 1, 2, 4, 8, 16, 32, 64, ...).
#error Change SERIAL_LINE_CONF_BUFSIZE in contiki-conf.h.
#endif

PROCESS_NAME(serial_shell_process);

PROCESS(serial_line_process, "serial driver", VCRTOS_CONFIG_MAIN_THREAD_STACK_SIZE);

process_event_t serial_line_event_message;

PROCESS_THREAD(serial_line_process, ev, data)
{
    (void) ev;

    static char c;
    static char buf[BUFSIZE];
    static int ptr;

    PROCESS_BEGIN();

    serial_line_event_message = process_alloc_event();
    ptr = 0;

    while (1)
    {
        if (vcstdio_read_available())
        {
            if (vcstdio_read((void *)&c, 1))
            {
                if (c == '\r' || c == '\n')
                {
                    printf("\r\n");
                    buf[ptr++] = (uint8_t)'\0';
                    process_post(&serial_shell_process, serial_line_event_message, buf);
                    ptr = 0;
                }
                else
                {
                    if (ptr < BUFSIZE - 1)
                    {
                        buf[ptr++] = c;
                        putchar(c);
                        fflush(stdout);
                    }
                }
            }
        }
        else
        {
            PROCESS_YIELD();
        }
    }

    PROCESS_END();
}

void serial_line_init(void)
{
    process_start(&serial_line_process, NULL);
}
