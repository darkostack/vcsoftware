set(unittest-includes ${unittest-includes}
)

set(unittest-sources
    ../../src/core/instance.cpp
    ../../src/core/thread.cpp
    stubs/cpu_stub.c
    stubs/thread_stub.c
    stubs/thread_arch_stub.c
)

set(unittest-test-sources
    src/core/thread/test_thread.cpp
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPROJECT_CORE_CONFIG_FILE='\"core-unittest-config.h\"'")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPROJECT_CORE_CONFIG_FILE='\"core-unittest-config.h\"'")
