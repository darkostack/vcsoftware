#ifndef MTOS_INSTANCE_H
#define MTOS_INSTANCE_H

#include <stdbool.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mtInstance mtInstance;

mtInstance *mtInstanceInit(void);

bool mtInstanceIsInitialized(mtInstance *aInstance);

mtInstance *mtInstanceGet(void);

const char *mtInstanceGetVersionString(void);

#ifdef __cplusplus
}
#endif

#endif /* MTOS_INSTANCE_H */
