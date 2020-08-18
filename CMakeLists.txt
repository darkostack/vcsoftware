include(CMakeForceCompiler)

cmake_minimum_required (VERSION 3.5)
set(CMAKE_SYSTEM_NAME Generic)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(vcsoftware_example)

if(VCSOFTWARE_SDK MATCHES vertexcom_vcrtos_vc7300)
    ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR})
    ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/source)

    set(VCSOFTWARE_COMMON_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/source/main.cpp
    )

    set(VERTEXCOM_VCRTOS_VC7300_SRCS
        ${VCSOFTWARE_COMMON_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/vertexcom_vcrtos_vc7300.cpp
    )

    CREATE_TEST_LIBRARY(vertexcom_vcrtos_vc7300 "${VERTEXCOM_VCRTOS_VC7300_SRCS}" "")
    add_dependencies(vertexcom_vcrtos_vc7300 PLATFORM_LIBS)
    target_link_libraries(vertexcom_vcrtos_vc7300 PLATFORM_LIBS)
endif()

if (VCSOFTWARE_SDK MATCHES vertexcom_vcrtos_linux)
    set(VERTEXCOM_VCRTOS_LINUX_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/vertexcom_vcrtos_linux.c
    )

    CREATE_TEST_LIBRARY(vertexcom_vcrtos_linux "${VERTEXCOM_VCRTOS_LINUX_SRCS}" "")
endif()

ADDSUBDIRS()
