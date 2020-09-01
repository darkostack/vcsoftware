include(CMakeForceCompiler)

cmake_minimum_required (VERSION 3.5)
set(CMAKE_SYSTEM_NAME Generic)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(vcsoftware_example)

ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR})
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/source)

set(VCSOFTWARE_COMMON_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/source/main.cpp
)

if(VCSOFTWARE_SDK MATCHES bare_vc7300)
    SET(BARE_VC7300_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/bare_vc7300.c
    )

    add_definitions(-DVCDRIVERS_PROJECT_CONFIG_FILE="vcdrivers-project-config.h")
    add_definitions(-DVCDRIVERS_VERSION="${VCSOFTWARE_OS_VER}")

    CREATE_TEST_LIBRARY(bare_vc7300 "${BARE_VC7300_SRCS}" "")
    add_dependencies(bare_vc7300 PLATFORM_LIBS)
    target_link_libraries(bare_vc7300 PLATFORM_LIBS)
endif()

if(VCSOFTWARE_SDK MATCHES vcrtos_contiki_linux)
    set(VCRTOS_CONTIKI_LINUX_SRCS
        ${VCSOFTWARE_COMMON_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos_contiki_linux.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos_processes.c
    )

    add_definitions(-DVCDRIVERS_PROJECT_CONFIG_FILE="vcdrivers-project-config.h")
    add_definitions(-DVCRTOS_PROJECT_CONFIG_FILE="vcrtos-project-config.h")
    add_definitions(-DVCRTOS_VERSION="${VCSOFTWARE_OS_VER}")

    CREATE_TEST_LIBRARY(vcrtos_contiki_linux "${VCRTOS_CONTIKI_LINUX_SRCS}" "")
    add_dependencies(vcrtos_contiki_linux PLATFORM_LIBS)
    target_link_libraries(vcrtos_contiki_linux PLATFORM_LIBS)
endif()

if(VCSOFTWARE_SDK MATCHES vcrtos_contiki_vc7300)
    set(VCRTOS_CONTIKI_VC7300_SRCS
        ${VCSOFTWARE_COMMON_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos_contiki_vc7300.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos_processes.c
    )

    add_definitions(-DVCDRIVERS_PROJECT_CONFIG_FILE="vcdrivers-project-config.h")
    add_definitions(-DVCRTOS_PROJECT_CONFIG_FILE="vcrtos-project-config.h")
    add_definitions(-DVCRTOS_VERSION="${VCSOFTWARE_OS_VER}")

    CREATE_TEST_LIBRARY(vcrtos_contiki_vc7300 "${VCRTOS_CONTIKI_VC7300_SRCS}" "")
    add_dependencies(vcrtos_contiki_vc7300 PLATFORM_LIBS)
    target_link_libraries(vcrtos_contiki_vc7300 PLATFORM_LIBS)
endif()

if(VCSOFTWARE_SDK MATCHES vcrtos_vc7300)
    set(VCRTOS_VC7300_SRCS
        ${VCSOFTWARE_COMMON_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos_vc7300.cpp
    )

    add_definitions(-DVCDRIVERS_PROJECT_CONFIG_FILE="vcdrivers-project-config.h")
    add_definitions(-DVCRTOS_PROJECT_CONFIG_FILE="vcrtos-project-config.h")
    add_definitions(-DVCRTOS_VERSION="${VCSOFTWARE_OS_VER}")

    CREATE_TEST_LIBRARY(vcrtos_vc7300 "${VCRTOS_VC7300_SRCS}" "")
    add_dependencies(vcrtos_vc7300 PLATFORM_LIBS)
    target_link_libraries(vcrtos_vc7300 PLATFORM_LIBS)
endif()

if (VCSOFTWARE_SDK MATCHES vcrtos_linux)
    set(VCRTOS_LINUX_SRCS
        ${VCSOFTWARE_COMMON_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/vcrtos_linux.cpp
    )

    add_definitions(-DVCDRIVERS_PROJECT_CONFIG_FILE="vcdrivers-project-config.h")
    add_definitions(-DVCRTOS_PROJECT_CONFIG_FILE="vcrtos-project-config.h")
    add_definitions(-DVCRTOS_VERSION="${VCSOFTWARE_OS_VER}")

    CREATE_TEST_LIBRARY(vcrtos_linux "${VCRTOS_LINUX_SRCS}" "")
    add_dependencies(vcrtos_linux PLATFORM_LIBS)
    target_link_libraries(vcrtos_linux PLATFORM_LIBS)
endif()

ADDSUBDIRS()
