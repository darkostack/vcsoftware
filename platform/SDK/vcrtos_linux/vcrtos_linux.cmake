# CROSS COMPILER SETTINGS
cmake_minimum_required (VERSION 3.5)
SET(CMAKE_SYSTEM_NAME Generic)

set(OS_BRAND Linux)

set(EXTRA_CMAKE_DIRS ${EXTRA_CMAKE_DIRS} "${CMAKE_SOURCE_DIR}/platform/SDK/vcrtos_linux")
