# Neon Kernel Quick-start guide

Neon is a collection of software components for real-time applications.

Neon Kernel is real-time cooperative, single stack kernel for very small, 
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

# Using Kernel

## Configuration

Configuration is done in `neon_app_config.h` header file. The file is included
by `base/include/shared/config.h` file, which is in included in all other Neon
components.

## Building

### Include paths

- `kernel/include` - standard Neon include path

### Source files

- `kernel/source/mm/heap.c` - Heap memory allocator
- `kernel/source/mm/mem.c` - Memory allocator class
- `kernel/source/mm/pool.c` - Pool memory allocator
- `kernel/source/mm/static.c` - Static memory allocator
- `kernel/source/sched/sched.c` - Scheduler
- `kernel/source/misc/timer.c` - Virtual timer
    
### Project dependencies

Neon Eds does depend on the following components:
- base


