#ifndef MTOS_THREAD_H
#define MTOS_THREAD_H

#include <mtos/kernel.h>
#include <mtos/cib.h>
#include <mtos/clist.h>
#include <mtos/msg.h>

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

typedef struct mtThread
{
    char *mSp;
    mtThreadStatus mStatus;
    uint8_t mPriority;
    mtKernelPid mPid;
    mtListNode mtRunQueueEntry;
    void *mWaitData;
    mtListNode mMsgWaiters;
    mtCib mMsgQueue;
    mtMsg *mMsgArray;
    char *mStackStart;
    const char *mName;
    int mStackSize;
} mtThread;

#ifdef __cplusplus
}
#endif

#endif /* MTOS_THREAD_H */
