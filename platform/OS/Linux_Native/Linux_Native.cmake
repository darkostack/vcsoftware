set (OS_BRAND Linux)

cmake_policy(SET CMP0003 NEW)
cmake_policy(SET CMP0011 OLD)

add_definitions(-DTARGET_IS_PC_LINUX)
add_definitions(-D__LINUX__)
