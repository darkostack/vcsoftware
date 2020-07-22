#include "test-helper.h"

static int isCpuInIsr = 0;
static int isPendSVInterruptTriggered = 0;

void testHelperSetCpuInIsr(void)
{
    isCpuInIsr = 1;
}

void testHelperResetCpuInIsr(void)
{
    isCpuInIsr = 0;
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

int mtCpuIsInIsr(void)
{
    return isCpuInIsr;
}

void mtCpuTriggerPendSVInterrupt(void)
{
    isPendSVInterruptTriggered = 1;
}
