#ifndef MTOS_CPU_H
#define MTOS_CPU_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

unsigned mtCpuIrqDisable(void);

unsigned mtCpuIrqEnable(void);

void mtCpuIrqRestore(unsigned aState);

int mtCpuIsInISR(void);

void mtCpuTriggerPendSVInterrupt(void);

#ifdef __cplusplus
}
#endif

#endif /* MTOS_CPU_H */
