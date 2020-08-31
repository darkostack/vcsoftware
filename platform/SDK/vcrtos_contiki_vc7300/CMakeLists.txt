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

ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/porting/vcrtos)
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/porting/arduino)

ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/porting/contiki-ng)
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/porting/contiki-ng/sys)

set(VCDRIVERS_COMMON_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/stdiobase.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/syscalls.c
)

set(VCDRIVERS_PERIPH_SRCS
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
)

set(VCRTOS_CORE_API_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/core/api/instance_api.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/core/api/msg_api.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/core/api/mutex_api.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/core/api/thread_api.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/core/api/event_api.cpp
)

set(VCRTOS_UTILS_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/utils/isrpipe.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/utils/ringbuffer.cpp
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
    ${CMAKE_CURRENT_SOURCE_DIR}/porting/contiki-ng/sys/process.c
    ${CMAKE_CURRENT_SOURCE_DIR}/porting/contiki-ng/sys/etimer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/porting/contiki-ng/sys/ctimer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/porting/contiki-ng/sys/timer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/porting/contiki-ng/clock.c
    ${CMAKE_CURRENT_SOURCE_DIR}/porting/contiki-ng/int-master.c
    ${CMAKE_CURRENT_SOURCE_DIR}/porting/contiki-ng/watchdog.c
)

set(PLATFORM_LIBS_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/dummy.c
)

add_definitions(-DVCDRIVERS_PROJECT_CONFIG_FILE="vcdrivers-project-config.h")
add_definitions(-DVCRTOS_PROJECT_CONFIG_FILE="vcrtos-project-config.h")
add_definitions(-DVCRTOS_VERSION="${VCSOFTWARE_OS_VER}")

add_library(vcdrivers_common STATIC ${VCDRIVERS_COMMON_SRCS})
add_library(vcdrivers_periph STATIC ${VCDRIVERS_PERIPH_SRCS})
add_library(vcrtos_core STATIC ${VCRTOS_CORE_SRCS})
add_library(vcrtos_core_api STATIC ${VCRTOS_CORE_API_SRCS})
add_library(vcrtos_utils STATIC ${VCRTOS_UTILS_SRCS})
add_library(vcrtos_cli STATIC ${VCRTOS_CLI_SRCS})
add_library(vcrtos_ztimer STATIC ${VCRTOS_ZTIMER_SRCS})
add_library(porting_vcrtos STATIC ${PORTING_VCRTOS_SRCS})
add_library(porting_arduino STATIC ${PORTING_ARDUINO_SRCS})
add_library(porting_contiki_ng STATIC ${PORTING_CONTIKI_NG_SRCS})
add_library(PLATFORM_LIBS STATIC ${PLATFORM_LIBS_SRCS})

set(PLATFORM_LIBS_list
    vcdrivers_common
    vcdrivers_periph
    vcrtos_core
    vcrtos_core_api
    vcrtos_utils
    vcrtos_cli
    vcrtos_ztimer
    porting_vcrtos
    porting_arduino
    porting_contiki_ng
)

add_dependencies(PLATFORM_LIBS ${PLATFORM_LIBS_list})

target_link_libraries(PLATFORM_LIBS -Wl,--whole-archive ${PLATFORM_LIBS_list} -Wl,--no-whole-archive)

