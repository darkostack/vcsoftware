# VCSOFTWARE (Vertexcom Software)

## Prerequisite

1. Install cmake
2. Install python

Set **ARMGCC_DIR** environment variable to tell the build system where is your compiler:  

bash script example to export ARMGCC_DIR variable:
```
#!/bin/bash

ARMGCC_DIR=/opt/toolchain/gcc-arm-none-eabi-6-2017-q2-update
export ARMGCC_DIR
```

All projects available are list under **platform/SDK** directory.


## bare_vc7300

Bare system (Non-RTOS) VC7300 (Sirius)

Deploy the source code and generate build system files:
```
python platform/vc-platform.py --verbose deploy --target=bare_vc7300 generate
```

Build the project:
```
cd __bare_vc7300

mkdir build && cd build

cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=./../platform/SDK/bare_vc7300/ARMGCC.cmake

make
```

#### Notes:

```

These following commands only need to run once:

python platform/vc-platform.py --verbose deploy --target=bare_vc7300 generate

cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=./../platform/SDK/bare_vc7300/ARMGCC.cmake

-------------------------------------------------------------------------------

deploy command will git all source code for the project,  

generate command will create __bare_vc7300 folder and CMAKE files to build the project.  

image would be in:

__${GENERATED_PROJECT_BUILD_FOLDER}/build/${CMAKE_BUILD_TYPE}/${PROJECT}.bin

in this case would be __bare_vc7300/build/Debug/bare_vc7300.bin

```


## freertos_vc7300

FreeRTOS minimal porting to VC7300 (Sirius)

Deploy the source code and generate build system files:
```
python platform/vc-platform.py --verbose deploy --target=freertos_vc7300 generate
```

Build the project:
```
cd __freertos_vc7300

mkdir build && cd build

cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=./../platform/SDK/freertos_vc7300/ARMGCC.cmake

make
```


## vcrtos_contiki_linux

RTOS version of contiki-ng with VCRTOS kernel running on Linux

Deploy the source code and generate build system files:
```
python platform/vc-platform.py --verbose deploy --target=vcrtos_contiki_linux generate
```

Build the project:
```
cd __vcrtos_contiki_linux

mkdir build && cd build

cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=./../platform/SDK/vcrtos_contiki_linux/GCC.cmake

make
```

Run the application
```
./Debug/vcrtos_contiki_linux.elf -i 0 -c /dev/tty
```

#### Notes:
Type **exit** on the command line if you want to exit the application,  
in some cases after exiting the application you may need to type **reset**  
to reset your terminal.


## vcrtos_contiki_vc7300

RTOS version of contiki-ng with VCRTOS kernel running on VC7300 (Sirius)

Deploy the source code and generate build system files:
```
python platform/vc-platform.py --verbose deploy --target=vcrtos_contiki_vc7300 generate
```

Build the project:
```
cd __vcrtos_contiki_vc7300

mkdir build && cd build

cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=./../platform/SDK/vcrtos_contiki_vc7300/ARMGCC.cmake

make
```

#### Notes:
Initial Mbed-Clien-PAL API will be implemented on this project.


## vcrtos_linux

VCRTOS kernel running on Linux

Deploy the source code and generate build system files:
```
python platform/vc-platform.py --verbose deploy --target=vcrtos_linux generate
```

Build the project:
```
cd __vcrtos_linux

mkdir build && cd build

cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=./../platform/SDK/vcrtos_linux/GCC.cmake

make
```

Run the application
```
./Debug/vcrtos_linux.elf -i 0 -c /dev/tty
```

#### Notes:
Type **exit** on the command line if you want to exit the application,  
in some cases after exiting the application you may need to type **reset**  
to reset your terminal.


## vcrtos_vc7300

VCRTOS kernel running on VC7300 (Sirius)

Deploy the source code and generate build system files:
```
python platform/vc-platform.py --verbose deploy --target=vcrtos_vc7300 generate
```

Build the project:
```
cd __vcrtos_vc7300

mkdir build && cd build

cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=./../platform/SDK/vcrtos_vc7300/ARMGCC.cmake

make
```

## vcrtos_wisun_vc7300

TODO
