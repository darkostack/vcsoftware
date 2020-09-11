#include <stdio.h>

#include <vcdrivers/stdiobase.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

extern int main(void);

void kernel_init(void)
{
    vcstdio_init(NULL);

    printf("freertos-%s started\r\n", FREERTOS_VERSION);

    TaskHandle_t xHandle = NULL;

    xTaskCreate((TaskFunction_t)main, "main", 1024, NULL, tskIDLE_PRIORITY, &xHandle);

    configASSERT(xHandle);

    vTaskStartScheduler();

    /* should not reach here */

    while(1);
}
