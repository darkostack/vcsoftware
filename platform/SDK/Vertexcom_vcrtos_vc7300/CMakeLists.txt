cmake_minimum_required (VERSION 3.5)
SET(CMAKE_SYSTEM_NAME Generic)

project(vertexcom_vcrtos_vc7300)

ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR})

ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/include)
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/include/cmsis)
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source)
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/sirius)

ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/include)
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source)

ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/porting/vcrtos)
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/porting/arduino)

set(VCDRIVERS_COMMON_SRCS
    #${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/assert_failure.c
    #${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/cortexm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/stdiobase.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/syscalls.c
)

set(VCDRIVERS_PERIPH_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/sirius/cpu_init.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/sirius/vectors.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/sirius/periph/gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/sirius/periph/irq.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/sirius/periph/tim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/sirius/periph/uart.c
)

set(VCRTOS_CORE_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/core/assert_failure.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/core/instance.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/core/msg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/core/mutex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/core/thread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/core/xtimer.cpp
)

set(VCRTOS_CORE_API_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/core/api/instance_api.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/core/api/msg_api.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/core/api/mutex_api.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/core/api/thread_api.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/core/api/xtimer_api.cpp
)

set(VCRTOS_UTILS_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/utils/isrpipe.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/utils/ringbuffer.cpp
)

set(VCRTOS_CLI_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/cli/cli.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/cli/cli_uart.cpp
)

set(PORTING_VCRTOS_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/porting/vcrtos/cortexm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/porting/vcrtos/cpu_arch.c
    ${CMAKE_CURRENT_SOURCE_DIR}/porting/vcrtos/kernel_init.c
    ${CMAKE_CURRENT_SOURCE_DIR}/porting/vcrtos/stdiobase_arch.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/porting/vcrtos/thread_arch.c
    ${CMAKE_CURRENT_SOURCE_DIR}/porting/vcrtos/xtimer_arch.c
)

set(PORTING_ARDUINO_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/porting/arduino/base.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/porting/arduino/serialport.cpp
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
add_library(porting_vcrtos STATIC ${PORTING_VCRTOS_SRCS})
add_library(porting_arduino STATIC ${PORTING_ARDUINO_SRCS})
add_library(PLATFORM_LIBS STATIC ${PLATFORM_LIBS_SRCS})

set(PLATFORM_LIBS_list
    vcdrivers_common
    vcdrivers_periph
    vcrtos_core
    vcrtos_core_api
    vcrtos_utils
    vcrtos_cli
    porting_vcrtos
    porting_arduino
)

add_dependencies(PLATFORM_LIBS ${PLATFORM_LIBS_list})

target_link_libraries(PLATFORM_LIBS -Wl,--whole-archive ${PLATFORM_LIBS_list} -Wl,--no-whole-archive)

