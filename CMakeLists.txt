include(CMakeForceCompiler)
# CROSS COMPILER SETTING
cmake_minimum_required (VERSION 3.5)
set(CMAKE_SYSTEM_NAME Generic)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(vcsoftwareExample)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/source)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/source/include)

# This is needed for compiling and linking SW.
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/source/platform")

FILE(GLOB VCSOFTWARE_EXAMPLE_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp"
)

CREATE_TEST_LIBRARY(vcsoftwareExample "${VCSOFTWARE_EXAMPLE_SRC}" "")

#add_dependencies(vcsoftwareExample todo)

ADDSUBDIRS()
