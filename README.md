

python platform/vc-platform.py deploy --target=x86_x64_NativeLinux_mbedtls generate

cd __x86_x64_NativeLinux_mbedtls

cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=./../platform/Toolchain/GCC/GCC.cmake


