# eSolid Real-Time Kernel

This is a bare-kernel, the result of two weeks of work. It is intended for a bigger 
project which already includes some synchronization mechanisms. The initial 
idea was that this kernel would only provide minimal functionality for context 
switching, but the kernel implementation went so nicely that I think that it 
would be great to share it. 

eSolid is a collection of resources for embedded system design and this
Real-Time Kernel is only a piece of that collection. Because of that fact
remember: __there are no synchronization or IPC mechanisms in this kernel__,
and it can be viewed as a preemptive Round-Robin scheduler, only.


## TODO list

- Integrate a profiling system (memory/stack usage, cpu usage...)
- Time delay/management
- test, test, test...


# Using eSolid Real-Time Kernel

## Configuration and ports

Configuration is done in two files: `kernel_cfg.h` (port independent settings) 
and in `cpu_cfg.h` (port depended settings).
Currently, kernel is ported only to ARMv7-M architecture range of 
microcontrollers. It was tested on STM32F100 series of microcontrollers, but it
should work, with minimal modifications, on any ARMv7-M CPU. Some other ports 
like AVR-GCC are planned, too.


## Building

The kernel was built using arm-none-eabi GCC v4.7.3 compiler toolchain (from 
https://launchpad.net/gcc-arm-embedded/+download) and binary was downloaded
to the MCU using _texane_ gdb-server. There are no makefiles, it is assumed
that IDE will generate them for you.

There are two source files which need to be compiled: 
- kernel.c in `/src` directory and 
- cpu.c in `/port/arm-none-eabi-gcc/v7-m` directory.

The following include paths are needed:
- `/inc`
- `/port/arm-none-eabi-gcc/common`
- `/port/arm-none-eabi-gcc/v7-m`

### Documentation

Some documentation is available under Wiki https://github.com/nradulovic/esolid-kernel/wiki. 
Doxygen configuration and full documentation source files are available in `/doc` directory. 
Go to the directory and run doxygen:

    # doxygen doxyfile-kernel
    # doxygen doxyfile-kernel-port

This will generate HTML, LaTex and man documentation in `/doc/out/kernel` and
`/doc/out/kernel-port` directories, respectively.


## Running
To successfully use and run kernel you will need to study the kernel 
documentation. The documentation is still being written and some examples will
be added later.
