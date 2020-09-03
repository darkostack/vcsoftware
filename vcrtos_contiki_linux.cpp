#include <stdio.h>

#include <vcrtos/instance.h>
#include <vcrtos/thread.h>
#include <vcrtos/cli.h>
#include <vcrtos/ztimer.h>

#include <vcdrivers/stdiobase.h>

#include "native_internal.h"
#include "tty_uart.h"

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

void cli_cmd_exit(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    tty_uart_close(VCDRIVERS_CONFIG_STDIOBASE_UART_DEV);

    real_exit(EXIT_SUCCESS);
}

const cli_command_t user_command_list[] = {
    { "exit", cli_cmd_exit },
};

void Main::setup(void)
{
    vcstdio_init(instance);

    vccli_uart_init(instance);
    vccli_set_user_commands(user_command_list, 1);

    process_init(instance);

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
