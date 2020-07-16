#ifndef MTOS_MSG_H
#define MTOS_MSG_H

#include <stdint.h>

#include <mtos/kernel.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mtMsg
{
    mtKernelPid mSenderPid;
    uint16_t mType;
    union
    {
        void *mPtr;
        uint32_t mValue;
    } mContent;
} mtMsg;

int mtMsgReceive(mtMsg *aMsg);

int mtMsgSend(mtMsg *aMsg, mtKernelPid aPid);

int mtMsgTrySend(mtMsg *aMsg, mtKernelPid aPid);

int mtMsgSendReceive(mtMsg *aMsg, mtMsg *aReply, mtKernelPid aPid);

int mtMsgReply(mtMsg *aMsg, mtMsg *aReply);

void mtMsgActiveThreadQueuePrint(void);

#ifdef __cplusplus
}
#endif

#endif /* MTOS_MSG_H */
