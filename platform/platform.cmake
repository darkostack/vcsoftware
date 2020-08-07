include(autogen.cmake)

if ("${CMAKE_TOOLCHAIN_FILE}" STREQUAL "")
    message(STATUS "Toolchain files directory is not set")
endif()
include(${CMAKE_TOOLCHAIN_FILE})

set(CMAKE_DEVICE_PATH ${CMAKE_SOURCE_DIR}/platform/Device/${VCSOFTWARE_DEVICE}/${VCSOFTWARE_DEVICE}.cmake)
if(EXISTS ${CMAKE_DEVICE_PATH})
        include(${CMAKE_DEVICE_PATH})
        message(STATUS "cmake file found for Device in ${CMAKE_DEVICE_PATH}")
else()
        message(STATUS "No cmake file found for Device in ${CMAKE_DEVICE_PATH}")
endif()
