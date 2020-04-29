#include "test-helper.h"

static int isCpuInISR = 0;
static int isPendSVInterruptTriggered = 0;

void testHelperSetCpuInISR(void)
{
    isCpuInISR = 1;
}

void testHelperResetCpuInISR(void)
{
    isCpuInISR = 0;
}

int testHelperIsPendSVInterruptTriggered(void)
{
    return isPendSVInterruptTriggered;
}

void testHelperResetPendSVTrigger(void)
{
    isPendSVInterruptTriggered = 0; 
}

unsigned mtCpuIrqDisable(void)
{
    return 0;
}

unsigned mtCpuIrqEnable(void)
{
    return 0;
}

void mtCpuIrqRestore(unsigned aState)
{
    (void) aState;
}

int mtCpuIsInISR(void)
{
    return isCpuInISR;
}

void mtCpuTriggerPendSVInterrupt(void)
{
    isPendSVInterruptTriggered = 1;
}
