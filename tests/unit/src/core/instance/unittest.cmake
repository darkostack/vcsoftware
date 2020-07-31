set(unittest-includes ${unittest-includes}
)

set(unittest-sources
    ../../src/core/instance.cpp
    ../../src/core/thread.cpp
)

set(unittest-test-sources
    src/core/instance/test_instance.cpp
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPROJECT_CONFIG_FILE='\"vcos-unittest-config.h\"'")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPROJECT_CONFIG_FILE='\"vcos-unittest-config.h\"'")
