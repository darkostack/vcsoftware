set(unittest-includes ${unittest-includes}
)

set(unittest-sources
    ../../src/core/instance.cpp
)

set(unittest-test-sources
    src/core/instance/test_instance.cpp
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DMTOS_PROJECT_CORE_CONFIG_FILE='\"mtos-core-unittest-config.h\"'")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMTOS_PROJECT_CORE_CONFIG_FILE='\"mtos-core-unittest-config.h\"'")
