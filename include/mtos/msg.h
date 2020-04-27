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

#ifdef __cplusplus
}
#endif

#endif /* MTOS_MSG_H */
