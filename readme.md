# Repository Description
This repository illustrates how C++ can be used for the firmware development in the embedded engineering projects. Using the STM32c031c6 as the target hardware platform, the projects show case the embedded c++ in a bare-metal context. Particularly, instead of using the common and straightforward way of developing through [StmcubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html), a combination of register level interfaces called [CMSIS](https://www.arm.com/technologies/cmsis), board files from the STM, and MAKE is used as the build system. Note: C++ developments are placed on top of the custom drivers that are created in C using the CMSIS register definitions. Note: NUCLEO-C031C6 is the part being used as the hardware platform, so, hardware dependencies for the firmware build should be updated if you are building for a different platform. THis is a work in progress and it's actively being updated!

**NOTE:** in order to check your developments using a snaity check, you can install the STM32CubeIDE to build and flash the projects. However, the imporatn thing is (at least what I experienced for MAC installation) there would be no examples or pre-made project after the IDE installtion. To get the examples, you would need to install the [STM32CubeC0 patch](https://github.com/STMicroelectronics/STM32CubeC0?utm_source=chatgpt.com).

## Build System
The build system that is used for the bare-metal development are copied over from the [STM32CubeC0 patch](https://www.st.com/en/embedded-software/stm32cubec0.html) . There a few more files that I had to copy over to make the build work out. Particularly, the files are:

    cmsis
    syscalls.c
    linker.ls
    starup.s

Another piece of the build system is the Makefile which enables us to output .elf file for programing via the compiler. In this regard, the tool chain we use to compile the output is the Arm GNU Toolchain that can be downloaded from [here](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) where the version I am using is this:

* arm-gnu-toolchain-15.2.rel1-darwin-arm64-arm-none-eabi.pkg

After, installation add the tool chain to the system path and make sure the tool chains is up and ready. You should see the info by putting this: 

`arm-none-eabi-gcc --version`.

## Flashing Tool
Since the openOCD does not support the board I am using,  I resorted to use the [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html).