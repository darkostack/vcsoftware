macro(SET_COMPILER_DBG_RLZ_FLAG flag value)
        SET(${flag}_DEBUG "${${flag}_DEBUG} ${value}")
        SET(${flag}_RELEASE "${${flag}_RELEASE} ${value}")
if (0)
 message("flag = ${flag}")
 message("value = ${value}")
 message("MY_C_FLAGS_RELEASE2 = ${CMAKE_C_FLAGS_RELEASE}")
endif()
endmacro(SET_COMPILER_DBG_RLZ_FLAG)

macro(SET_COMPILER_DBG_RLZ_COMMON_FLAG flag value)
        SET(${flag}_DEBUG "${${flag}_DEBUG} ${${value}_DEBUG}")
        SET(${flag}_RELEASE "${${flag}_RELEASE} ${${value}_RELEASE}")
endmacro(SET_COMPILER_DBG_RLZ_COMMON_FLAG)

SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_CXX_FLAGS "")

#SET(CMAKE_FLAGS_COMMON_DEBUG "-DDEBUG -g -Og")
SET(CMAKE_FLAGS_COMMON_DEBUG "-g Og")
SET(CMAKE_FLAGS_COMMON_RELEASE "-Og")

SET(CMAKE_COMPILE_FLAGS_BASIS "-mcpu=cortex-m3 -mthumb -mfloat-abi=soft -ffunction-sections -fdata-sections -Wunused -Wuninitialized -Wall -Wextra -Wshadow -Wlogical-op -Waggregate-return -MMD -MP")

### Set ASM flags ###
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "${CMAKE_COMPILE_FLAGS_BASIS}")
SET_COMPILER_DBG_RLZ_COMMON_FLAG (CMAKE_ASM_FLAGS CMAKE_FLAGS_COMMON)

### Set C/C++ flags ###
message("${CMAKE_SOURCE_DIR}/include_file.txt")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "@${CMAKE_SOURCE_DIR}/include_file.txt")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_CXX_FLAGS "@${CMAKE_SOURCE_DIR}/include_file.txt")

SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "${CMAKE_COMPILE_FLAGS_BASIS} -std=gnu99")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_CXX_FLAGS "${CMAKE_COMPILE_FLAGS_BASIS}")
SET_COMPILER_DBG_RLZ_COMMON_FLAG (CMAKE_C_FLAGS CMAKE_FLAGS_COMMON)
SET_COMPILER_DBG_RLZ_COMMON_FLAG (CMAKE_CXX_FLAGS CMAKE_FLAGS_COMMON)

### Set linker flags ###
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-Xlinker --gc-sections --specs=nosys.specs")

