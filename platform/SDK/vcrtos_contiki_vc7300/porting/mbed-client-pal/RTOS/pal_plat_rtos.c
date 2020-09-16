#include <vcrtos/config.h>
#include <vcrtos/cpu.h>
#include <vcrtos/instance.h>
#include <vcrtos/thread.h>

#if VCRTOS_CONFIG_ZTIMER_ENABLE
#include <vcrtos/ztimer.h>
#endif

#include "pal.h"
#include "pal_plat_rtos.h"
#include <stdlib.h>

#define TRACE_GROUP "PAL"

//! Timer structure
typedef struct palTimer {
    palTimerID_t timerID;
    palTimerFuncPtr function;
    void *functionArgs;
    uint32_t timerType;
} palTimer_t;

typedef struct palMutex {
    palMutexID_t mutexID;
} palMutex_t;

typedef struct palSemaphore {
    palSemaphoreID_t semaphoreID;
    uint32_t maxCount;
} palSemaphore_t;

typedef struct palThreadData {
    palThreadFuncPtr userFunction;
    void *userFunctionArgument;
    kernel_pid_t sysThreadID;
} palThreadData_t;

#define PAL_MAX_CONCURRENT_THREADS 8

//PAL_PRIVATE palMutexID_t g_threadsMutex = NULLPTR;
//PAL_PRIVATE palThreadData_t *g_threadsArray[PAL_MAX_CONCURRENT_THREADS] = { 0 };

//PAL_PRIVATE void threadFree(palThreadData_t** threadData);

palStatus_t pal_plat_osDelay(uint32_t milliseconds)
{
    ztimer_sleep(ZTIMER_USEC, milliseconds * 1000);
    return PAL_SUCCESS;
}

uint64_t pal_plat_osKernelSysTick()
{
    return ztimer_now(ZTIMER_USEC);
}

uint64_t pal_plat_osKernelSysTickMicroSec(uint64_t microseconds)
{
    return microseconds;
}

uint64_t pal_plat_osKernelSysTickFrequency()
{
    return 1000000; /* 1 MHz */
}

void *pal_plat_malloc(size_t len)
{
    (void) len;
    return NULL;
}

void pal_plat_free(void *buffer)
{
    (void) buffer;
}
