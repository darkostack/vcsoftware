cmake_minimum_required (VERSION 3.5)
SET(CMAKE_SYSTEM_NAME Generic)

project(vertexcom_vc7300)

ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR})
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/include)
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/include/cmsis)
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source)
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/sirius)
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/include)
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/vcrtos/source)
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/vcrtos_port)

set(VCDRIVERS_COMMON_SRCS
    #${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/assert_failure.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/cortexm.c
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
)

set(VCRTOS_PORT_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos_port/thread_arch.c
)

set(PLATFORM_LIBS_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/dummy.c
)

add_definitions(-DPROJECT_CONFIG_FILE="vcrtos-project-config.h")

add_library(vcdrivers_common STATIC ${VCDRIVERS_COMMON_SRCS})
add_library(vcdrivers_periph STATIC ${VCDRIVERS_PERIPH_SRCS})
add_library(vcrtos_core STATIC ${VCRTOS_CORE_SRCS})
add_library(vcrtos_core_api STATIC ${VCRTOS_CORE_API_SRCS})
add_library(vcrtos_port STATIC ${VCRTOS_PORT_SRCS})
add_library(PLATFORM_LIBS STATIC ${PLATFORM_LIBS_SRCS})

set(PLATFORM_LIBS_list
    vcdrivers_common
    vcdrivers_periph
    vcrtos_core
    vcrtos_core_api
    vcrtos_port
)

add_dependencies(PLATFORM_LIBS ${PLATFORM_LIBS_list})

target_link_libraries(PLATFORM_LIBS -Wl,--whole-archive ${PLATFORM_LIBS_list} -Wl,--no-whole-archive)

