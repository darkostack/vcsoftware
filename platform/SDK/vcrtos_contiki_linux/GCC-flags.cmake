include(CMakeForceCompiler)

macro(SET_COMPILER_DBG_RLZ_FLAG flag value)
    set(${flag}_DEBUG "${${flag}_DEBUG} ${value}")
    set(${flag}_RELEASE "${${flag}_RELEASE} ${value}")
#enable this if for debugging
if (0)
 message("flag = ${flag}")
 message("value = ${value}")
 message("MY_C_FLAGS_RELEASE2 = ${CMAKE_C_FLAGS_RELEASE}")
endif(0) # comment end
endmacro(SET_COMPILER_DBG_RLZ_FLAG)

macro(SET_COMPILER_DBG_RLZ_COMMON_FLAG flag value)
    set(${flag}_DEBUG "${${flag}_DEBUG} ${${value}_DEBUG}")
    set(${flag}_RELEASE "${${flag}_RELEASE} ${${value}_RELEASE}")
endmacro(SET_COMPILER_DBG_RLZ_COMMON_FLAG)

SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_CXX_FLAGS "")

set(CMAKE_FLAGS_COMMON_DEBUG "-DDEBUG -g -Og")
set(CMAKE_FLAGS_COMMON_RELEASE "-Og")

set(CMAKE_COMPILE_FLAGS_BASIS "-fno-common -ffunction-sections -fdata-sections -Wunused -Wuninitialized -Wall -Wextra -Wshadow -Wlogical-op -Waggregate-return -MD -MP")

SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "${CMAKE_COMPILE_FLAGS_BASIS}")
SET_COMPILER_DBG_RLZ_COMMON_FLAG (CMAKE_ASM_FLAGS CMAKE_FLAGS_COMMON)

message("${CMAKE_SOURCE_DIR}/include_file.txt")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "@${CMAKE_SOURCE_DIR}/include_file.txt")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_CXX_FLAGS "@${CMAKE_SOURCE_DIR}/include_file.txt")

SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-m32 -std=gnu99 ${CMAKE_COMPILE_FLAGS_BASIS}")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_CXX_FLAGS "-m32 ${CMAKE_COMPILE_FLAGS_BASIS}")
SET_COMPILER_DBG_RLZ_COMMON_FLAG (CMAKE_C_FLAGS CMAKE_FLAGS_COMMON)
SET_COMPILER_DBG_RLZ_COMMON_FLAG (CMAKE_CXX_FLAGS CMAKE_FLAGS_COMMON)

SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-Xlinker --gc-sections")

message(STATUS "BUILD_TYPE: " ${CMAKE_BUILD_TYPE})
