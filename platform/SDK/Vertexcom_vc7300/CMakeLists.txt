cmake_minimum_required (VERSION 3.5)
SET(CMAKE_SYSTEM_NAME Generic)

project(vertexcomRTOS)

ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/include)
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/include/cmsis)
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source)
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/sirius)

set(vcdrivers_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/assert_failure.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/stdiobase.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/sirius/cortexm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/sirius/cpu.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/sirius/syscalls.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/sirius/vectors.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/sirius/periph/gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/sirius/periph/irq.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/sirius/periph/tim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vcdrivers/source/sirius/periph/uart.c
)

set(PLATFORM_LIBS_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/dummy.c
)

add_library(vcdrivers STATIC ${vcdrivers_SRCS})

add_library(PLATFORM_LIBS STATIC ${PLATFORM_LIBS_SRCS})

set(PLATFORM_LIBS_list vcdrivers)
add_dependencies(PLATFORM_LIBS ${PLATFORM_LIBS_list})

target_link_libraries(PLATFORM_LIBS -Wl,--whole-archive ${PLATFORM_LIBS_list} -Wl,--no-whole-archive)

