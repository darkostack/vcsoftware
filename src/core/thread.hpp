#ifndef CORE_THREAD_HPP
#define CORE_THREAD_HPP

#include <mtos/config.h>
#include <mtos/thread.h>
#include <mtos/stat.h>
#include <mtos/cpu.h>

#include "core/msg.hpp"
#include "core/cib.hpp"
#include "core/clist.hpp"

namespace mt {

class ThreadScheduler;

class Instance;

#if !MTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
extern uint64_t gInstanceRaw[];
#endif

class Thread : public mtThread
{
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

    static int IsPidValid(mtKernelPid aPid);

    void InitMsgQueue(Msg *aArg, int aNum);

    int QueuedMsg(Msg *aMsg);

    int GetNumOfMsgInQueue(void);

    int HasMsgQueue(void);

private:
    void InitRunqueueEntry(void) { mRunqueueEntry.mNext = NULL; }

    void InitMsg(void);

    void SetStackStart(char *aPtr) { mStackStart = aPtr; }

    void SetStackSize(int aStackSize) { mStackSize = aStackSize; }

    void SetName(const char *aName) { mName = aName; }

    void SetStackPointer(char *aPtr) { mStackPointer = aPtr; }

    void StackInit(mtThreadHandlerFunc aHandlerFunc, void *aArg, void *aStackStart, int aStackSize);

    template <typename Type> inline Type &Get(void) const; 

#if MTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
    Instance &GetInstance(void) const { return *static_cast<Instance *>(mInstance); }
#else
    Instance &GetInstance(void) const { return *reinterpret_cast<Instance *>(&gInstanceRaw); }
#endif
};

class ThreadScheduler : public Clist
{
public:
    ThreadScheduler(void)
        : mNumOfThreadsInScheduler(0)
        , mContextSwitchRequestFromIsr(0)
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

    unsigned int IsContextSwitchRequestedFromIsr(void) { return mContextSwitchRequestFromIsr; }

    void EnableContextSwitchRequestFromIsr(void) { mContextSwitchRequestFromIsr = 1; }

    void DisableContextSwitchRequestFromIsr(void) { mContextSwitchRequestFromIsr = 0; }

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

    static const char *ThreadStatusToString(mtThreadStatus aStatus);

private:
    uint32_t GetRunqueueBitCache(void) { return mRunqueueBitCache; }

    void SetRunqueueBitCache(uint8_t aPriority) { mRunqueueBitCache |= 1 << aPriority; }

    void ResetRunqueueBitCache(uint8_t aPriority) { mRunqueueBitCache &= ~(1 << aPriority); }

    Thread *GetNextThreadFromRunqueue(void);

    uint8_t GetLSBIndexFromRunqueue(void);

    int mNumOfThreadsInScheduler;

    unsigned int mContextSwitchRequestFromIsr;

    Thread *mScheduledThreads[KERNEL_PID_LAST + 1];

    Thread *mCurrentActiveThread;

    mtKernelPid mCurrentActivePid;

    Clist mSchedulerRunqueue[MTOS_CONFIG_THREAD_PRIORITY_LEVELS];

    uint32_t mRunqueueBitCache;

    mtSchedulerStat mThreadsSchedulerStat[KERNEL_PID_LAST + 1];
};

} // namespace mt

#endif /* CORE_THREAD_HPP */
