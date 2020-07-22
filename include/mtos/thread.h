#ifndef MTOS_THREAD_H
#define MTOS_THREAD_H

#include <mtos/config.h>
#include <mtos/kernel.h>
#include <mtos/cib.h>
#include <mtos/clist.h>
#include <mtos/msg.h>
#include <mtos/instance.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *(*mtThreadHandlerFunc)(void *aArg);

typedef enum
{
    THREAD_STATUS_STOPPED,
    THREAD_STATUS_SLEEPING,
    THREAD_STATUS_MUTEX_BLOCKED,
    THREAD_STATUS_RECEIVE_BLOCKED,
    THREAD_STATUS_SEND_BLOCKED,
    THREAD_STATUS_REPLY_BLOCKED,
    THREAD_STATUS_FLAG_BLOCKED_ANY,
    THREAD_STATUS_FLAG_BLOCKED_ALL,
    THREAD_STATUS_MBOX_BLOCKED,
    THREAD_STATUS_COND_BLOCKED,
    THREAD_STATUS_RUNNING,
    THREAD_STATUS_PENDING,
    THREAD_STATUS_NUMOF
} mtThreadStatus;

#define THREAD_STATUS_NOT_FOUND ((mtThreadStatus)-1)

typedef struct mtThread
{
    char *mStackPointer;
    mtThreadStatus mStatus;
    uint8_t mPriority;
    mtKernelPid mPid;
    mtListNode mRunqueueEntry;
    void *mWaitData;
    mtListNode mMsgWaiters;
    mtCib mMsgQueue;
    mtMsg *mMsgArray;
    char *mStackStart;
    const char *mName;
    int mStackSize;
#if MTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
    void *mInstance;
#endif
} mtThread;

#define THREAD_FLAGS_CREATE_SLEEPING (0x1)
#define THREAD_FLAGS_CREATE_WOUT_YIELD (0x2)
#define THREAD_FLAGS_CREATE_STACKMARKER (0x4)

void mtThreadSchedulerRun(mtInstance *aInstance);

void mtThreadTaskExit(void);

int mtThreadPidIsValid(mtKernelPid aPid);

void mtThreadYield(void);

mtThread *mtThreadCurrent(mtInstance *aInstance);

mtKernelPid mtThreadCurrentPid(void);

char *mtThreadArchStackInit(mtThreadHandlerFunc aFunction, void *aArg, void *aStackStart, int aStackSize);

void mtThreadArchStackPrint(void);

int mtThreadArchStackUsage(void);

void *mtThreadArchIsrStackPointer(void);

void *mtThreadArchStackStart(void);

#ifdef __cplusplus
}
#endif

#endif /* MTOS_THREAD_H */
