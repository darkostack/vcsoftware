#ifndef MTOS_MUTEX_H
#define MTOS_MUTEX_H

#include <mtos/list.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MUTEX_LOCKED ((mtListNode *)-1)

typedef struct mtMutex
{
    mtListNode mQueue;
} mtMutex;

void mtMutexLock(mtMutex *aMutex);

void mtMutexUnlock(mtMutex *aMutex);

#ifdef __cplusplus
}
#endif

#endif /* MTOS_MUTEX_H */
