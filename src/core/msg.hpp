#ifndef CORE_MSG_HPP
#define CORE_MSG_HPP

#include <stdint.h>
#include <assert.h>

#include <mtos/config.h>
#include <mtos/kernel.h>
#include <mtos/msg.h>

#include "core/list.hpp"

namespace mt {

class Thread;

class Instance;

#if !MTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
extern uint64_t gInstanceRaw[];
#endif

class Msg : public mtMsg
{
public:
    Msg(void) {}

    explicit Msg(Instance &aInstance)
    {
        Init(aInstance);
    }

    void Init(Instance &aInstance)
    {
#if MTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
        mInstance = static_cast<void *>(&aInstance);
#else
        (void)aInstance;
#endif
        mSenderPid = KERNEL_PID_UNDEF;
        mType = 0;
        mContent.mPtr = NULL;
        mContent.mValue = 0;
    }

    int QueuedMsg(Thread *aTarget);

    int Send(mtKernelPid aTargetPid);

    int TrySend(mtKernelPid aTargetPid);

    int SendToSelfQueue(void);

    int SendInIsr(mtKernelPid aTargetPid);

    int IsSentByIsr(void) { return mSenderPid == KERNEL_PID_ISR; }

    int Receive(void) { return Receive(1); }

    int TryReceive(void) { return Receive(0); }

    int SendReceive(Msg *aReply, mtKernelPid aTargetPid);

    int Reply(Msg *aReply);

    int ReplyInIsr(Msg *aReply);

    template <typename Type> inline Type &Get(void) const; 

private:
    int Send(mtKernelPid aTargetPid, int aBlocking, unsigned aState);

    int Receive(int aBlocking);

#if MTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
    Instance &GetInstance(void) const { return *static_cast<Instance *>(mInstance); }
#else
    Instance &GetInstance(void) const { return *reinterpret_cast<Instance *>(&gInstanceRaw); }
#endif
};

} // namespace mt

#endif /* CORE_MSG_HPP */
