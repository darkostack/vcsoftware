include(CMakeForceCompiler)

cmake_minimum_required (VERSION 3.5)
set(CMAKE_SYSTEM_NAME Generic)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(vcsoftware_example)

ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR})
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/source)

FILE(GLOB VCSOFTWARE_EXAMPLE_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp"
)

CREATE_TEST_LIBRARY(vcsoftware_example "${VCSOFTWARE_EXAMPLE_SRC}" "")

add_dependencies(vcsoftware_example PLATFORM_LIBS)

target_link_libraries(vcsoftware_example PLATFORM_LIBS)

ADDSUBDIRS()
