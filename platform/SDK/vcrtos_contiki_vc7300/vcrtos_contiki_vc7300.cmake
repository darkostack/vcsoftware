# CROSS COMPILER SETTINGS
cmake_minimum_required (VERSION 3.5)
SET(CMAKE_SYSTEM_NAME Generic)

set(OS_BRAND vcrtos)

set(CMAKE_EXE_LINKER_FLAGS_DEBUG
    "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -T${CMAKE_SOURCE_DIR}/platform/SDK/${VCSOFTWARE_SDK}/${VCSOFTWARE_DEVICE}.ld -static"
)

set(CMAKE_EXE_LINKER_FLAGS_RELEASE
    "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -T${CMAKE_SOURCE_DIR}/platform/SDK/${VCSOFTWARE_SDK}/${VCSOFTWARE_DEVICE}.ld -static"
)

set(EXTRA_CMAKE_DIRS ${EXTRA_CMAKE_DIRS} "${CMAKE_SOURCE_DIR}/platform/SDK/${VCSOFTWARE_SDK}")
