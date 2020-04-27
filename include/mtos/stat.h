#ifndef MTOS_STAT_H
#define MTOS_STAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mtSchedulerStat
{
    uint32_t mLastStart;
    unsigned int mSchedules;
    uint64_t mRuntimeTicks;
} mtSchedulerStat;

#ifdef __cplusplus
}
#endif

#endif /* MTOS_STAT_H */
