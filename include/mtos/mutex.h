#ifndef MTOS_MUTEX_H
#define MTOS_MUTEX_H

#include <mtos/config.h>
#include <mtos/list.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MUTEX_LOCKED ((mtListNode *)-1)

typedef struct mtMutex
{
    mtListNode mQueue;
#if MTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
    void *mInstance;
#endif
} mtMutex;

void mtMutexLock(mtMutex *aMutex);

void mtMutexUnlock(mtMutex *aMutex);

#ifdef __cplusplus
}
#endif

#endif /* MTOS_MUTEX_H */
