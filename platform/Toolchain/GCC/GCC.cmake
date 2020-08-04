INCLUDE(CMakeForceCompiler)

#This file defines needed options for native GCC compiler.

# TOOLCHAIN EXTENSION
IF(WIN32)
    SET(TOOLCHAIN_EXT ".exe")
ELSE()
    SET(TOOLCHAIN_EXT "")
ENDIF()


