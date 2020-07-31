#ifndef VCOS_MSG_H
#define VCOS_MSG_H

#include <stdint.h>

#include <vcos/config.h>
#include <vcos/kernel.h>
#include <vcos/list.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct msg
{
    kernel_pid_t sender_pid;
    uint16_t type;
    union
    {
        void *ptr;
        uint32_t value;
    } content;
#if VCOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
    void *instance;
#endif
} msg_t;

typedef struct msg_bus
{
    list_node_t subs;
    uint16_t id;
} msg_bus_t;

typedef struct msg_bus_entry
{
    list_node_t next;
    uint32_t event_mask;
    kernel_pid_t pid;
} msg_bus_entry_t;

int msg_receive(msg_t *msg);

int msg_send(msg_t *msg, kernel_pid_t pid);

int msg_try_send(msg_t *msg, kernel_pid_t pid);

int msg_send_receive(msg_t *msg, msg_t *reply, kernel_pid_t pid);

int msg_reply(msg_t *msg, msg_t *reply);

void msg_active_thread_queue_print(void);

#ifdef __cplusplus
}
#endif

#endif /* VCOS_MSG_H */
