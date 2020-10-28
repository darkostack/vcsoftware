#include "contiki.h"
#include "shell.h"

#include "dev/serial-line.h"

#include <stdio.h>
#include <string.h>

PROCESS(serial_shell_process, "serial shell", VCRTOS_CONFIG_MAIN_THREAD_STACK_SIZE);

void shell_default_output(const char *text1, int len1, const char *text2, int len2)
{
    int i;

    if (text1 == NULL)
    {
        text1 = "";
        len1 = 0;
    }

    if (text2 == NULL)
    {
        text2 = "";
        len2 = 0;
    }

    for (i = 0; i < len1; i++)
    {
        printf("%c", text1[i]);
    }

    for (i = 0; i < len2; i++)
    {
        printf("%c", text2[i]);
    }

    printf("\r\n");
}

void shell_prompt(char *str)
{
    printf("%s\r\n", str);
}

void shell_exit(void)
{
}

PROCESS_THREAD(serial_shell_process, ev, data)
{
    PROCESS_BEGIN();

    shell_init();

    while (1)
    {
        PROCESS_WAIT_EVENT_UNTIL(ev == serial_line_event_message && data != NULL);
        shell_input(data, strlen(data));
    }

    PROCESS_END();
}

void serial_shell_init(void)
{
    process_start(&serial_shell_process, NULL);
}
