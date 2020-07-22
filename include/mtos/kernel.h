#ifndef MTOS_KERNEL_H
#define MTOS_KERNEL_H

#include <stdint.h>

#include <mtos/config.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KERNEL_MAXTHREADS (32)

#define KERNEL_PID_UNDEF (0)
#define KERNEL_PID_FIRST (KERNEL_PID_UNDEF + 1)
#define KERNEL_PID_LAST (KERNEL_PID_FIRST + KERNEL_MAXTHREADS - 1)

#define KERNEL_PID_ISR (KERNEL_PID_LAST - 1)

#define KERNEL_THREAD_PRIORITY_LEVELS MTOS_CONFIG_THREAD_PRIORITY_LEVELS
#define KERNEL_THREAD_PRIORITY_MIN (KERNEL_THREAD_PRIORITY_LEVELS - 1)
#define KERNEL_THREAD_PRIORITY_IDLE KERNEL_THREAD_PRIORITY_MIN
#define KERNEL_THREAD_PRIORITY_MAIN (KERNEL_THREAD_PRIORITY_MIN - (KERNEL_THREAD_PRIORITY_LEVELS / 2))

#define PRIkernel_pid PRIi16

typedef int16_t mtKernelPid;

void mtKernelInit(void);

#ifdef __cplusplus
}
#endif

#endif /* MTOS_KERNEL_H */
