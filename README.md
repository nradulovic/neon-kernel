# Neon RT Kernel 

Neon RT Kernel is real-time cooperative, single stack kernel for very small, 
memory constrained embedded systems. 


## Features


### System

* **Static design**. No heap memory required. All data structures are allocated 
    during the compile time.
* **Static configuration**. The configuration is done in C header files using macro
    defines.
* **Lightweight**. All kernel functions have very small code size and usage of
    stack located and static variables is at minimum.
* **Portable**. The kernel can run on wide range of micro-controllers. It was tested
    on 8-bit Microchip PIC up to 64-bit AMD CPU as Linux user thread.
* **Easy integration**. The kernel can be easily integrated into an existing 
    project. The kernel does not require any custom linker scripts or start-up 
    files. Assembly code is minimally used in port system. 
* **Preemption aware**. The system can be coupled with an existing RTOS to 
    provide preemptive execution if it is needed. Several instances of the 
    scheduler can be started to provide multiple levels of preemption.


### Scheduler

* Unlimited number of threads.
* Up to 64 priority levels on 8-bit micro-controllers and up to 256 priority 
    levels on 32-bit micro-controllers.
* Round-robing, non-preemptive scheduling of threads with same priority.
* O(1) constant time complexity, scheduling time does not increase if new 
    threads are added to scheduler's ready queue.
* Hardware support for efficient schedulers algorithm execution.


### Array queues



### Time management



## Usage


### Configuration

Configuration is done in two files: `nkernel_config.h` (port independent 
settings) and in `port_config.h` (port depended settings, located in port 
directory structure).

Configuration option `CONFIG_NUM_OF_NC_TASKS` is used to specify the maximum 
amount of concurrent threads in the system. Note that threads can be deleted.
So if the system is not executing all threads at the same time then this number
can be set to the maximum number of active threads in order to save RAM memory.

Configuration option `CONFIG_PRIORITY_LEVELS` is used to specify the number 
of thread priority levels. It is preferred that this configuration option is 
held below or equal to 8 on low end 8-bit micro-controllers. Higher number of levels 
may impact the execution performance on low end 8-bit micro-controllers.


### Ports

Ports are divided into three sections:
* **Architecture**. This port responsibility is to manage CPU, interrupt 
    controller, core power manager and system timer.
* **Platform**. Port platform manages C language specifics (compiler 
    abstraction) and provides critical code section protection.
* **Family**. Several port families can have the same port architecture. Each 
    port family introduces diversity to port architecture. 


### System

To initialize the system user application must call `nkernel_init()` function.
This function will prepare internal data structure for scheduler.


### Thread

A thread is a function with the following prototype: 

        void function(void * stack)
    
Each thread **must return** after some defined time. When the thread returns it 
leaves the CPU time for other threads to execute. Ideally, threads are written 
as finite state machines which by definition are always returning.

During the thread execution interrupts are allowed. 

The argument to thread function is always the stack pointer which was given 
during the thread creation process. This gives the ability to write parametrized 
thread functions.


### Creating a thread

A new thread is created using `nthread_create()` function. The function searches
through free thread pool to obtain a thread data structure.

1. First parameter is pointer to thread entry function.
2. Second parameter is pointer to thread stack space. This parameter is optional 
and it is needed when writing parametrized thread functions.
3. Third parameter is thread priority. The higher the number the higher the 
importance of the thread. The maximum priority level is defined by 
`CONFIG_PRIORITY_LEVELS` configuration option. More than one threads can have
same priority.

After the thread is created the thread is in `NSTATE_IDLE` state. To put the 
thread in ready/running state use `nthread_ready()` function.


### Destroying a thread

A thread is destroyed by using `nthread_destroy()` function. If the thread is 
ready for execution or is currently executing then it will be removed from ready 
queue. When the thread is destroyed its data structure is returned to free 
thread pool.


### Running threads

The threads are invoked by scheduler. To start the scheduler call the kernel 
function `nkernel_start()`. The scheduler will evaluate all threads that are 
ready and schedule them for execution. When there are no threads ready for 
execution the scheduler function will return.

Threads can be created and destroyed during the system execution.


## Building

The kernel was built using arm-none-eabi GCC v4.8 compiler tool-chain (from 
https://launchpad.net/gcc-arm-embedded/+download) and binary was downloaded
to the MCU using _texane_ gdb-server. There are no makefiles, it is assumed
that IDE will generate them for you.


### Example for STM32F10x family port

Currently, kernel is officially ported to ARMv7-M architecture range of 
micro-controllers. It was tested on STM32F100 series of micro-controllers, but 
it should work on any ARMv7-M CPU, with minimal modifications. Some other ports 
like AVR-GCC are planned, too.

There are two groups of source files which need to be compiled for ARMv7-M 
architecture: 
- kernel.c and debug.c in `./source` source directory and 
- port C sources in `./port/arm-none-eabi-gcc/v7-m` port directory.

The following include paths are needed:
- `./include`
- `./port/arm-none-eabi-gcc/v7-m`
- `./port/arm-none-eabi-gcc/common`
- `./port/arm-none-eabi-gcc/stm32f10x`


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
