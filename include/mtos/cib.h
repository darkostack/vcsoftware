#ifndef MTOS_CIB_H
#define MTOS_CIB_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mtCib
{
    unsigned int mReadCount;
    unsigned int mWriteCount;
    unsigned int mMask;
} mtCib;

#ifdef __cplusplus
}
#endif

#endif /* MTOS_CIB_H */
