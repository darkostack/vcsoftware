#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

void kernel_malloc_failed(void)
{
    printf("ERROR: kernel malloc failed\n");
    taskDISABLE_INTERRUPTS();
    for ( ;; );
}

void kernel_stack_overflow(void)
{
    printf("ERROR: kernel stack overflow\n");
    taskENABLE_INTERRUPTS();
    for ( ;; );
}
