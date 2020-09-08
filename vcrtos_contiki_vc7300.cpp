#include <arduino/base.hpp>

#include <vcrtos/instance.h>
#include <vcrtos/thread.h>
#include <vcrtos/ztimer.h>
#include <vcrtos/cli.h>

#include "main.hpp"

extern "C" {
#include "contiki.h"
#include "contiki-net.h"
#include "net/ipv6/uip-ds6.h"
#include "net/linkaddr.h"
#include "net/mac/mac.h"
#include "net/mac/framer/frame802154.h"
#include "sys/node-id.h"
#include "lib/random.h"

#include "sys/log.h"
#define LOG_MODULE "Main"
#define LOG_LEVEL LOG_LEVEL_MAIN
}

extern "C" void vcrtos_cmd_ps(int argc, char **argv);

const cli_command_t user_command_list[] = {
    { "ps", vcrtos_cmd_ps },
};

void Main::setup(void)
{
    vccli_uart_init(instance);
    vccli_set_user_commands(user_command_list, 1);

    process_init(instance);

    ctimer_init();

    uint16_t uid = 0x1234;

    linkaddr_node_addr.u8[LINKADDR_SIZE - 1] = (uid & 0xff);
    linkaddr_node_addr.u8[LINKADDR_SIZE - 2] = (uid >> 8);

    netstack_init();
    node_id_init();

    random_init(node_id);

    LOG_INFO("- Routing: %s\r\n", NETSTACK_ROUTING.name);
    LOG_INFO("- Net: %s\r\n", NETSTACK_NETWORK.name);
    LOG_INFO("- MAC: %s\r\n", NETSTACK_MAC.name);
    LOG_INFO("- 802.15.4 PANID: 0x%04x\r\n", IEEE802154_PANID);
    LOG_INFO("- 802.15.4 Default channel: %u\r\n", IEEE802154_DEFAULT_CHANNEL);
    LOG_INFO("- Link-layer address: ");
    LOG_INFO_LLADDR(&linkaddr_node_addr);
    LOG_INFO_("\r\n");

#if NETSTACK_CONF_WITH_IPV6
    {
        uip_ds6_addr_t *lladdr;
        memcpy(&uip_lladdr.addr, &linkaddr_node_addr, sizeof(uip_lladdr.addr));
        process_start(&tcpip_process, NULL);
        lladdr = uip_ds6_get_link_local(-1);
        LOG_INFO("- Tentative link-local IPv6 address: ");
        LOG_INFO_6ADDR(lladdr != NULL ? &lladdr->ipaddr : NULL);
        LOG_INFO_("\r\n");
    }
#endif

    autostart_start(autostart_processes);
}

void Main::loop(void)
{
    thread_sleep(instance);
}
