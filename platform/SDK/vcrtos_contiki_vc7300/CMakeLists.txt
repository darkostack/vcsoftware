cmake_minimum_required (VERSION 3.5)
SET(CMAKE_SYSTEM_NAME Generic)

project(vcrtos_contiki_vc7300)

ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR})

ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/include)
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/include/cmsis)
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source)
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/sirius)

ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/include)
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source)

ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os)
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/sys)

ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/mbed-client-pal/Source)
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/mbed-client-pal/Source/PAL-Impl)
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/mbed-client-pal/Source/PAL-Impl/Services-API)
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/mbed-client-pal/Source/port/Platform-API)
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/mbed-trace)

ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/porting/vcrtos)
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/porting/arduino)
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/porting/contiki-ng)
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/porting/mbed-client-pal)

set(VCDRIVERS_COMMON_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/stdiobase.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/syscalls.c
)

set(VCDRIVERS_CPU_PERIPH_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/sirius/cpu.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/sirius/vectors.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/sirius/periph/gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/sirius/periph/irq.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/sirius/periph/tim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/sirius/periph/uart.c
)

set(VCRTOS_CORE_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/core/instance.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/core/msg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/core/mutex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/core/thread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/core/rmutex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/core/sema.c
)

set(VCRTOS_CORE_API_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/core/api/instance_api.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/core/api/msg_api.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/core/api/mutex_api.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/core/api/thread_api.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/core/api/event_api.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/core/api/heap_api.cpp
)

set(VCRTOS_UTILS_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/utils/isrpipe.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/utils/ringbuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/utils/heap.cpp
)

set(VCRTOS_CLI_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/cli/cli.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/cli/cli_uart.cpp
)

set(VCRTOS_ZTIMER_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/ztimer/core.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/ztimer/overhead.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/ztimer/util.c
)

set(CONTIKI_NG_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/sys/process.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/sys/etimer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/sys/ctimer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/sys/stimer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/sys/timer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/sys/autostart.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/sys/log.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/sys/node-id.c

    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/lib/list.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/lib/memb.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/lib/random.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/lib/assert.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/lib/ccm-star.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/lib/aes-128.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/lib/crc16.c

    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/dev/nullradio.c

    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/link-stats.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/linkaddr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/nbr-table.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/net-debug.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/packetbuf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/queuebuf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/netstack.c

    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/nullnet/nullnet.c

    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/routing/nullrouting/nullrouting.c

    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/ipv6/sicslowpan.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/ipv6/simple-udp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/ipv6/tcp-socket.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/ipv6/tcpip.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/ipv6/udp-socket.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/ipv6/uip-ds6-nbr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/ipv6/uip-ds6-route.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/ipv6/uip-ds6.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/ipv6/uip-icmp6.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/ipv6/uip-nameserver.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/ipv6/uip-nd6.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/ipv6/uip-packetqueue.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/ipv6/uip-sr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/ipv6/uip-udp-packet.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/ipv6/uip6.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/ipv6/uipbuf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/ipv6/uiplib.c

    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/routing/rpl-classic/rpl-dag-root.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/routing/rpl-classic/rpl-dag.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/routing/rpl-classic/rpl-ext-header.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/routing/rpl-classic/rpl-icmp6.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/routing/rpl-classic/rpl-mrhof.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/routing/rpl-classic/rpl-nbr-policy.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/routing/rpl-classic/rpl-of0.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/routing/rpl-classic/rpl-timers.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/routing/rpl-classic/rpl.c

    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/mac/mac-sequence.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/mac/mac.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/mac/framer/frame802154.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/mac/framer/framer-802154.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/mac/framer/nullframer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/mac/csma/csma-output.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/mac/csma/csma-security.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/mac/csma/csma.c
    ${CMAKE_CURRENT_SOURCE_DIR}/contiki-ng/os/net/mac/nullmac/nullmac.c
)

set(PORTING_VCRTOS_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/porting/vcrtos/cortexm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/porting/vcrtos/cpu_arch.c
    ${CMAKE_CURRENT_SOURCE_DIR}/porting/vcrtos/kernel_init.c
    ${CMAKE_CURRENT_SOURCE_DIR}/porting/vcrtos/stdiobase_arch.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/porting/vcrtos/thread_arch.c
    ${CMAKE_CURRENT_SOURCE_DIR}/porting/vcrtos/commands.c
    ${CMAKE_CURRENT_SOURCE_DIR}/porting/vcrtos/ztimer_periph.c
    ${CMAKE_CURRENT_SOURCE_DIR}/porting/vcrtos/assert_failure.c
    ${CMAKE_CURRENT_SOURCE_DIR}/porting/vcrtos/simple_malloc.c
)

set(PORTING_ARDUINO_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/porting/arduino/base.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/porting/arduino/serialport.cpp
)

set(PORTING_CONTIKI_NG_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/porting/contiki-ng/clock.c
    ${CMAKE_CURRENT_SOURCE_DIR}/porting/contiki-ng/int-master.c
    ${CMAKE_CURRENT_SOURCE_DIR}/porting/contiki-ng/watchdog.c
)

set(PORTING_MBED_CLIENT_PAL_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/porting/mbed-client-pal/RTOS/pal_plat_rtos.c
)

set(PLATFORM_LIBS_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/dummy.c
)

add_definitions(-DVCDRIVERS_PROJECT_CONFIG_FILE="vcdrivers-project-config.h")
add_definitions(-DVCRTOS_PROJECT_CONFIG_FILE="vcrtos-project-config.h")
add_definitions(-DVCRTOS_VERSION="${VCSOFTWARE_OS_VER}")

add_definitions(-DPAL_PLATFORM_DEFINED_CONFIGURATION="vcrtos-project-config.h")

add_definitions(-DMAC_CONF_WITH_CSMA=1)
add_definitions(-DNETSTACK_CONF_WITH_IPV6=1)
add_definitions(-DROUTING_CONF_RPL_CLASSIC=1)

add_library(vcdrivers_common STATIC ${VCDRIVERS_COMMON_SRCS})
add_library(vcdrivers_cpu_periph STATIC ${VCDRIVERS_CPU_PERIPH_SRCS})
add_library(vcrtos_core STATIC ${VCRTOS_CORE_SRCS})
add_library(vcrtos_core_api STATIC ${VCRTOS_CORE_API_SRCS})
add_library(vcrtos_utils STATIC ${VCRTOS_UTILS_SRCS})
add_library(vcrtos_cli STATIC ${VCRTOS_CLI_SRCS})
add_library(vcrtos_ztimer STATIC ${VCRTOS_ZTIMER_SRCS})
add_library(contiki_ng STATIC ${CONTIKI_NG_SRCS})
add_library(porting_vcrtos STATIC ${PORTING_VCRTOS_SRCS})
add_library(porting_arduino STATIC ${PORTING_ARDUINO_SRCS})
add_library(porting_contiki_ng STATIC ${PORTING_CONTIKI_NG_SRCS})
add_library(porting_mbed_client_pal STATIC ${PORTING_MBED_CLIENT_PAL_SRCS})
add_library(PLATFORM_LIBS STATIC ${PLATFORM_LIBS_SRCS})

set(PLATFORM_LIBS_list
    vcdrivers_common
    vcdrivers_cpu_periph
    vcrtos_core
    vcrtos_core_api
    vcrtos_utils
    vcrtos_cli
    vcrtos_ztimer
    contiki_ng
    porting_vcrtos
    porting_arduino
    porting_contiki_ng
)

add_dependencies(PLATFORM_LIBS ${PLATFORM_LIBS_list})

target_link_libraries(PLATFORM_LIBS -Wl,--whole-archive ${PLATFORM_LIBS_list} -Wl,--no-whole-archive)

