set(CPU_NAME sirius)

set(ENABLE_BUILD_IMAGE OFF CACHE STRING "avoid building vcdrivers image")

include_directories("${CMAKE_SOURCE_DIR}/platform/Middleware/vcdrivers")
include_directories("${CMAKE_SOURCE_DIR}/platform/Middleware/vcdrivers/vcdrivers/include")
include_directories("${CMAKE_SOURCE_DIR}/platform/Middleware/vcdrivers/vcdrivers/include/cmsis")
include_directories("${CMAKE_SOURCE_DIR}/platform/Middleware/vcdrivers/vcdrivers/source/sirius")

set(EXTRA_CMAKE_DIRS ${EXTRA_CMAKE_DIRS} "${CMAKE_SOURCE_DIR}/platform/Middleware/vcdrivers/vcdrivers")

list (APPEND SRC_LIBS vcdrivers_common vcdrivers_sirius vcdrivers_sirius_periph)
