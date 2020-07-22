set(unittest-includes ${unittest-includes}
)

set(unittest-sources
    ../../src/core/instance.cpp
    ../../src/core/thread.cpp
    ../../src/core/mutex.cpp
    ../../src/core/msg.cpp
    stubs/assert_api_stub.c
    stubs/cpu_stub.c
    stubs/thread_stub.c
    stubs/thread_arch_stub.c
)

set(unittest-test-sources
    src/core/msg/test_msg.cpp
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPROJECT_CONFIG_FILE='\"mtos-unittest-config.h\"'")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPROJECT_CONFIG_FILE='\"mtos-unittest-config.h\"'")
