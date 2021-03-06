cmake_minimum_required (VERSION 3.5)
SET(CMAKE_SYSTEM_NAME Generic)

project(freertos_vc7300)

ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR})

ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/include)
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/include/cmsis)
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source)
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/sirius)

ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/amazon-freertos/freertos_kernel)
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/amazon-freertos/freertos_kernel/include)
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/amazon-freertos/freertos_kernel/portable/GCC/ARM_CM3)

set(VCDRIVERS_COMMON_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/stdiobase.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/syscalls.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/assert_failure.c
)

set(VCDRIVERS_CPU_PERIPH_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/sirius/vectors.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/sirius/periph/gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/sirius/periph/irq.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/sirius/periph/tim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/sirius/periph/uart.c
)

set(FREERTOS_KERNEL_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/amazon-freertos/freertos_kernel/croutine.c
    ${CMAKE_CURRENT_SOURCE_DIR}/amazon-freertos/freertos_kernel/event_groups.c
    ${CMAKE_CURRENT_SOURCE_DIR}/amazon-freertos/freertos_kernel/list.c
    ${CMAKE_CURRENT_SOURCE_DIR}/amazon-freertos/freertos_kernel/queue.c
    ${CMAKE_CURRENT_SOURCE_DIR}/amazon-freertos/freertos_kernel/tasks.c
    ${CMAKE_CURRENT_SOURCE_DIR}/amazon-freertos/freertos_kernel/timers.c
)

set(FREERTOS_PORT_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/amazon-freertos/freertos_kernel/portable/GCC/ARM_CM3/port.c
    ${CMAKE_CURRENT_SOURCE_DIR}/amazon-freertos/freertos_kernel/portable/MemMang/heap_4.c
    ${CMAKE_CURRENT_SOURCE_DIR}/freertos_hook.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cortexm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel_init.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cpu.c
)

set(PLATFORM_LIBS_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/dummy.c
)

add_definitions(-DVCDRIVERS_PROJECT_CONFIG_FILE="vcdrivers-project-config.h")
add_definitions(-DFREERTOS_VERSION="${VCSOFTWARE_OS_VER}")

add_library(vcdrivers_common STATIC ${VCDRIVERS_COMMON_SRCS})
add_library(vcdrivers_cpu_periph STATIC ${VCDRIVERS_CPU_PERIPH_SRCS})
add_library(freertos_kernel STATIC ${FREERTOS_KERNEL_SRCS})
add_library(freertos_port STATIC ${FREERTOS_PORT_SRCS})
add_library(PLATFORM_LIBS STATIC ${PLATFORM_LIBS_SRCS})

set(PLATFORM_LIBS_list
    vcdrivers_common
    vcdrivers_cpu_periph
    freertos_kernel
    freertos_port
)

add_dependencies(PLATFORM_LIBS ${PLATFORM_LIBS_list})

target_link_libraries(PLATFORM_LIBS -Wl,--whole-archive ${PLATFORM_LIBS_list} -Wl,--no-whole-archive)

