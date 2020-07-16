#ifndef CORE_THREAD_HPP
#define CORE_THREAD_HPP

#include "core/config.h"

#include <mtos/thread.h>
#include <mtos/stat.h>
#include <mtos/cpu.h>

#include "core/msg.hpp"
#include "core/cib.hpp"
#include "core/clist.hpp"
#include "core/locator.hpp"

namespace mt {

class ThreadScheduler;

class Thread : public mtThread, public InstanceLocatorInit
{
    friend class ThreadScheduler;

public:
    static Thread *Init(Instance &aInstance,
                        char *aStack,
                        int aStackSize,
                        char aPriority,
                        int aFlags,
                        mtThreadHandlerFunc aHandlerFunc,
                        void *aArg,
                        const char *aName);

    mtKernelPid GetPid(void) { return mPid; }

    void SetPid(mtKernelPid aPid) { mPid = aPid; }

    mtThreadStatus GetStatus(void) { return mStatus; }

    void SetStatus(mtThreadStatus aStatus) { mStatus = aStatus; }

    uint8_t GetPriority(void) { return mPriority; }

    void SetPriority(uint8_t aPriority) { mPriority = aPriority; }

    mtListNode *GetRunqueueEntry(void) { return &mRunqueueEntry; }

    const char *GetName(void) { return mName; }

    void AddToList(List *aList);

    static Thread *GetThreadPointerFromItsListMember(List *aList);

    void InitMsgQueue(Msg *aArg, int aNum);

private:
    void InitRunqueueEntry(void) { mRunqueueEntry.mNext = NULL; }

    void InitMsg(void);

    void SetStackStart(char *aPtr) { mStackStart = aPtr; }

    void SetStackSize(int aStackSize) { mStackSize = aStackSize; }

    void SetName(const char *aName) { mName = aName; }

    void SetStackPointer(char *aPtr) { mStackPointer = aPtr; }

    void StackInit(mtThreadHandlerFunc aHandlerFunc, void *aArg, void *aStackStart, int aStackSize);
};

class ThreadScheduler : public Clist
{
public:
    ThreadScheduler(void)
        : mNumOfThreadsInScheduler(0)
        , mContextSwitchRequestFromISR(0)
        , mCurrentActiveThread(NULL)
        , mCurrentActivePid(KERNEL_PID_UNDEF)
        , mRunqueueBitCache(0)
    {
        for (mtKernelPid i = KERNEL_PID_FIRST; i <= KERNEL_PID_LAST; ++i)
        {
            mScheduledThreads[i] = NULL;
        }
    }

    Thread *GetThreadFromScheduler(mtKernelPid aPid) { return mScheduledThreads[aPid]; }

    void SetThreadToScheduler(Thread *aThread, mtKernelPid aPid) { mScheduledThreads[aPid] = aThread; }

    unsigned int IsContextSwitchRequestedFromISR(void) { return mContextSwitchRequestFromISR; }

    void EnableContextSwitchRequestFromISR(void) { mContextSwitchRequestFromISR = 1; }

    void DisableContextSwitchRequestFromISR(void) { mContextSwitchRequestFromISR = 0; }

    int GetNumOfThreadsInScheduler(void) { return mNumOfThreadsInScheduler; }

    void IncrementNumOfThreadsInScheduler(void) { mNumOfThreadsInScheduler++; }

    void DecrementNumOfThreadsInScheduler(void) { mNumOfThreadsInScheduler--; }

    Thread *GetCurrentActiveThread(void) { return mCurrentActiveThread; }

    void SetCurrentActiveThread(Thread *aThread) { mCurrentActiveThread = aThread; }

    mtKernelPid GetCurrentActivePid(void) { return mCurrentActivePid; }

    void SetCurrentActivePid(mtKernelPid aPid) { mCurrentActivePid = aPid; }

    void Run(void);

    void SetThreadStatusAndUpdateRunqueue(Thread *aThread, mtThreadStatus aStatus);

    void ContextSwitch(uint8_t aPriority);

    void SleepingCurrentThread(void);

    int WakeupThread(mtKernelPid aPid);

    void Yield(void);

    static void YieldHigherPriorityThread(void);

    const char *ThreadStatusToString(mtThreadStatus aStatus);

private:
    uint32_t GetRunqueueBitCache(void) { return mRunqueueBitCache; }

    void SetRunqueueBitCache(uint8_t aPriority) { mRunqueueBitCache |= 1 << aPriority; }

    void ResetRunqueueBitCache(uint8_t aPriority) { mRunqueueBitCache &= ~(1 << aPriority); }

    Thread *GetNextThreadFromRunqueue(void);

    uint8_t GetLSBIndexFromRunqueue(void);

    int mNumOfThreadsInScheduler;

    unsigned int mContextSwitchRequestFromISR;

    Thread *mScheduledThreads[KERNEL_PID_LAST + 1];

    Thread *mCurrentActiveThread;

    mtKernelPid mCurrentActivePid;

    Clist mSchedulerRunqueue[MTOS_CONFIG_THREAD_SCHED_PRIO_LEVELS];

    uint32_t mRunqueueBitCache;

    mtSchedulerStat mThreadsSchedulerStat[KERNEL_PID_LAST + 1];
};

} // namespace mt

#endif /* CORE_THREAD_HPP */
