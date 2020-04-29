#ifndef TEST_HELPER_H
#define TEST_HELPER_H

/* unit test helper functions */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void testHelperSetCpuInISR(void);

void testHelperResetCpuInISR(void);

int testHelperIsPendSVInterruptTriggered(void);

void testHelperResetPendSVTrigger(void);

#ifdef __cplusplus
}
#endif

#endif /* TEST_HELPER_H */
