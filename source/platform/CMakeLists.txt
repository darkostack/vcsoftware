INCLUDE(CMakeForceCompiler)

cmake_minimum_required (VERSION 3.5)
SET(CMAKE_SYSTEM_NAME Generic)

project(platformCommon)

ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/include)
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/${OS_BRAND})

FILE(GLOB PLATFORM_SRC 
    "${CMAKE_CURRENT_SOURCE_DIR}/${OS_BRAND}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/${OS_BRAND}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/${OS_BRAND}/*.cpp"
)

# Library for providing the mcc_* functions.
CREATE_LIBRARY(platformCommon "${PLATFORM_SRC}" "")
