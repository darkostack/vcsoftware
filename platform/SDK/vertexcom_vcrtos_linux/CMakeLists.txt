cmake_minimum_required (VERSION 3.5)
SET(CMAKE_SYSTEM_NAME Generic)

project(vertexcom_vcrtos_linux)

ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR})

ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/include)
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source)

ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/include)

set(NATIVE_COMMON_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/tramp.S
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel_init.c
    ${CMAKE_CURRENT_SOURCE_DIR}/irq_cpu.c
    ${CMAKE_CURRENT_SOURCE_DIR}/native_cpu.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syscalls.c
    ${CMAKE_CURRENT_SOURCE_DIR}/startup.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stdio_native.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/async_read.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ztimer_periph.c
)

set_property(SOURCE tramp.S PROPERTY LANGUAGE C)

set(NATIVE_PERIPH_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/periph/uart.c
    ${CMAKE_CURRENT_SOURCE_DIR}/periph/gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/periph/tim.c
)

set(VCRTOS_CORE_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/core/assert_failure.c
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

set(VCRTOS_ZTIMER_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/ztimer/core.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/ztimer/overhead.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/ztimer/util.c
)

set(VCRTOS_CLI_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/cli/cli.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source/cli/cli_uart.cpp
)

set(PLATFORM_LIBS_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/dummy.c
)

add_definitions(-DVCDRIVERS_PROJECT_CONFIG_FILE="vcdrivers-project-config.h")
add_definitions(-DVCRTOS_PROJECT_CONFIG_FILE="vcrtos-project-config.h")
add_definitions(-DVCRTOS_VERSION="${VCSOFTWARE_OS_VER}")

add_library(native_common STATIC ${NATIVE_COMMON_SRCS})
add_library(native_periph STATIC ${NATIVE_PERIPH_SRCS})
add_library(vcrtos_core STATIC ${VCRTOS_CORE_SRCS})
add_library(vcrtos_core_api STATIC ${VCRTOS_CORE_API_SRCS})
add_library(vcrtos_utils STATIC ${VCRTOS_UTILS_SRCS})
add_library(vcrtos_ztimer STATIC ${VCRTOS_ZTIMER_SRCS})
add_library(vcrtos_cli STATIC ${VCRTOS_CLI_SRCS})
add_library(PLATFORM_LIBS STATIC ${PLATFORM_LIBS_SRCS})

set(PLATFORM_LIBS_list
    native_common
    native_periph
    vcrtos_core
    vcrtos_core_api
    vcrtos_utils
    vcrtos_ztimer
    vcrtos_cli
)

add_dependencies(PLATFORM_LIBS ${PLATFORM_LIBS_list})

target_link_libraries(PLATFORM_LIBS -Wl,--whole-archive ${PLATFORM_LIBS_list} -ldl -Wl,--no-whole-archive)
