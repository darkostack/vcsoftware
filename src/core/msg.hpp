#ifndef CORE_MSG_HPP
#define CORE_MSG_HPP

#include <stdint.h>

#include <mtos/kernel.h>
#include <mtos/msg.h>

#include "core/list.hpp"
#include "core/locator.hpp"

namespace mt {

class Thread;

class Msg : public mtMsg, public List, public InstanceLocatorInit
{
public:
    explicit Msg(void)
    {
        mSenderPid = KERNEL_PID_UNDEF;
        mType = 0;
        mContent.mPtr = NULL;
        mContent.mValue = 0;
    }

    int QueuedMsg(Thread *aTarget);

    int Send(mtKernelPid aTargetPid);

    int TrySend(mtKernelPid aTargetPid);

    int SendToSelf(void);

    int SendInISR(mtKernelPid aTargetPid);

    int IsSentByISR(void) { return mSenderPid == KERNEL_PID_ISR; }

    int Receive(void) { return Receive(1); }

    int TryReceive(void) { return Receive(0); }

    int SendReceive(Msg *aReply, mtKernelPid aTargetPid);

    int Reply(Msg *aReply);

    int ReplyInISR(Msg *aReply);

    int Available(void);

private:
    int Send(mtKernelPid aTargetPid, int aBlocking, unsigned aState);

    int Receive(int aBlocking);
};

} // namespace mt

#endif /* CORE_MSG_HPP */
