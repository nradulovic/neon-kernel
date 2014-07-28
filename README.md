# Introduction 

Neon RT Kernel is real-time cooperative, single stack kernel for embedded 
systems. 

## TODO list

- Integrate a profiling system (memory/stack usage, CPU usage...)
- test, test, test...

# Features

## System
1. Static design; no heap memory required. All data structures are allocated 
    during the compile time.
2. Static configuration, no execution overhead.
3. Portable. The kernel can run on wide range of micro-controllers. It was tested
    on 8-bit Microchip PIC up to 64-bit AMD CPU as Linux user thread.
4. Can be easily integrated into an existing system. The kernel does not require
    custom linker scripts or start files. Assembly code is minimally used in
    port system.


## Scheduler
1. Unlimited number of tasks.
2. Up to 64 priority levels on 8-bit micro-controllers and up to 256 priority 
    levels on 32-bit micro-controllers.
3. Round-robing, non-preemptive scheduling of tasks with same priority.
4. O(1) constant time complexity, scheduling time does not increase if new tasks 
    are added.


# Using eSolid - Real-Time Kernel

## Configuration and ports

Configuration is done in two files: `nkernel_config.h` (port independent 
settings) and in `port_config.h` (port depended settings, located in port 
directory structure).

Currently, kernel is officially ported only to ARMv7-M architecture range of 
micro-controllers. It was tested on STM32F100 series of micro-controllers, but it
should work, with minimal modifications, on any ARMv7-M CPU. Some other ports 
like AVR-GCC are planned, too.


## Building

The kernel was built using arm-none-eabi GCC v4.8 compiler tool-chain (from 
https://launchpad.net/gcc-arm-embedded/+download) and binary was downloaded
to the MCU using _texane_ gdb-server. There are no makefiles, it is assumed
that IDE will generate them for you.


#### Example for STM32F10x family port

There are two groups of source files which need to be compiled for ARMv7-M 
architecture: 
- kernel.c and debug.c in `./source` source directory and 
- port C sources in `./port/arm-none-eabi-gcc/v7-m` port directory.

The following include paths are needed:
- `./include`
- `./port/arm-none-eabi-gcc/v7-m`

## Documentation

Some documentation is available under Wiki 
https://github.com/nradulovic/esolid-kernel/wiki. 
Doxygen configuration and full documentation source files are available in `/doc` 
directory. Go to the directory `doc` create a directory named `kernel` and than 
run doxygen:

    # doxygen doxyfile-kernel

This will generate HTML, LaTex and man documentation in `./doc/kernel` directory.


## Running
To successfully use and run kernel you will need to study the kernel 
documentation. The documentation is still being written and some examples will
be added later.
