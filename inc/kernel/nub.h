/*
 * This file is part of NUB Real-Time Kernel.
 *
 * Copyright (C) 2010 - 2014 Nenad Radulovic
 *
 * NUB Real-Time Kernel is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * NUB Real-Time Kernel is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with NUB Real-Time Kernel.  If not, see <http://www.gnu.org/licenses/>.
 *
 * web site:    http://github.com/nradulovic
 * e-mail  :    nenad.b.radulovic@gmail.com
 *//***************************************************************************************************************//**
 * @file
 * @author  	Nenad Radulovic
 * @brief       Main kernel interface
 * @defgroup    kernel Kernel
 * @brief       Kernel overview
 * @details     Main kernel interface is divided into several sections.
 *************************************************************************************************************//** @{ */
/**@defgroup    kernel_intf Interface
 * @brief       Kernel main interface
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

#ifndef NUB_H_
#define NUB_H_

/*=================================================================================================  INCLUDE FILES  ==*/

#include <stdint.h>

#include "plat/compiler.h"
#include "arch/cpu.h"
#include "kernel/nub_config.h"
#include "kernel/ndebug.h"
#include "kernel/nbitop.h"
#include "kernel/nlist.h"

/*=======================================================================================================  MACRO's  ==*/

/*----------------------------------------------------------------------------------------------------------------*//**
 * @defgroup    kernel_generic Kernel generic services and identification
 * @brief       Kernel generic services and unique identification
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

/**@brief       Identifies kernel major version number
 */
#define NSYS_VER_MAJOR                      (1ul)

/**@brief       Identifies kernel minor version number
 */
#define NSYS_VER_MINOR                      (0ul)

/**@brief       Identifies kernel patch level
 */
#define NSYS_VER_PATCH                      (0ul)

/**@brief       Identifies the underlying kernel version number
 */
#define NSYS_VER                            (((NSYS_VER_MAJOR) << 24) | (NSYS_VER_MINOR << 16) | (NSYS_VER_PATCH))

/**@brief       Kernel identification string
 */
#define NSYS_ID                             "NUB Real-Time Kernel"

/**@} *//*--------------------------------------------------------------------------------------------------------*//**
 * @defgroup    thread Thread
 * @brief       Thread management services
 * @details     For more details see @ref threads.
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

/**@brief       Converts the required stack elements into the stack array index.
 * @param       elem
 *              Number of stack elements: the stack size is expressed in number of elements regardless of the size of
 *              port general purpose registers.
 * @return      Number of stack elements needed for stack usage.
 */
#define NSTACK_SIZE(elem)                   (PORT_STACK_SIZE(elem))

/**@brief       Maximum level of priority possible for application thread
 */
#define NTHREAD_PRIORITY_MAX                (CONFIG_PRIORITY_LEVELS - 1u)

/**@brief       Minimum level of priority possible for application thread
 */
#define NTHREAD_PRIORITY_MIN                (0u)

/**@} *//*--------------------------------------------------------------------------------------------------------*//**
 * @defgroup    semaphore
 * @brief       Semaphore
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

#define NSEM_INITIALIZER(sem)

/**@} *//*--------------------------------------------------------------------------------------  C++ extern begin  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*====================================================================================================  DATA TYPES  ==*/

/*----------------------------------------------------------------------------------------------------------------*//**
 * @defgroup    prio_array Priority array
 * @brief       Priority array allows efficient insertion and removal of threads in and out from a thread run_queue.
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

/**@brief       Priority array structure
 * @details     An priority array consists of an array of sub-queues. There is one sub-run_queue per priority level. Each
 *              sub-run_queue contains the runnable threads at the corresponding priority level. There is also a bitmap
 *              corresponding to the array that is used to determine effectively the highest-priority task on the run_queue.
 * @notapi
 */
struct nub_prio_array
{
#if (CONFIG_PRIORITY_BUCKETS != 1)
    /**@brief       Priority Bit Map structure
     * @notapi
     */
    struct nub_prio_bitmap
    {
#if   (CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH) || defined(__DOXYGEN__)
        natomic                     bitGroup;                                   /**<@brief Bit group indicator        */
#endif  /* (CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH) */
        natomic                     bit[NDIVISION_ROUNDUP(CONFIG_PRIORITY_BUCKETS, NCPU_DATA_WIDTH)];
                                                                                /**<@brief Bucket indicator           */
    }                           bitmap;                                         /**<@brief Priority bitmap            */
#endif  /* (CONFIG_PRIORITY_BUCKETS != 1) */
    struct ndlist               sentinel[CONFIG_PRIORITY_BUCKETS];
};

/** @} *//*-------------------------------------------------------------------------------------------------------*//**
 * @addtogroup  kernel_generic
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

/**@brief       Scheduler state enumeration
 * @details     For more details see: @ref states
 * @api
 */
enum nsys_state
{
    NSYS_RUN                    = (0x0u << 0),                                  /**<@brief Executing threads          */
    NSYS_ISR                    = (0x1u << 0),                                  /**<@brief Servicing an interrupt     */
    NSYS_LOCK                   = (0x1u << 1),                                  /**<@brief Locked state               */
    NSYS_ISR_LOCK               = (NSYS_LOCK | NSYS_ISR),                       /**<@brief Locked while in interrupt  */
    NSYS_SLEEP                  = (0x1u << 2),                                  /**<@brief Sleeping                   */
    NSYS_INIT                   = (0x1u << 3),                                  /**<@brief Initialization state       */
    NSYS_INACTIVE               = (0x1u << 4)                                   /**<@brief Scheduler is not active    */
};

/**@brief       Scheduler state type
* @api
*/
typedef enum nsys_state nsys_state;


enum nsys_status
{
    N_SUCCESS                   =   0u,                                         /**<@brief Operation is successful    */
    N_E_OBJ_REMOVED             = 100u                                          /**<@brief Error: object removed      */
};

typedef enum nsys_status nsys_status;

/**@brief       System context structure
 * @details     This structure holds important status data for the kernel.
 * @notapi
 */
struct nub_sys_ctx
{
    struct nub_prio_array       run_queue;
    struct nthread *            cthread;                                        /**<@brief The current thread         */
    struct nthread *            pthread;                                        /**<@brief The pending thread         */
    enum nsys_state             state;                                          /**<@brief Current scheduler state    */
    uint_fast8_t                lock_count;
};

/** @} *//*-------------------------------------------------------------------------------------------------------*//**
 * @addtogroup  thread
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

/**@brief       Thread structure
 * @details     A thread structure is a data structure used by kernel to maintain information about a thread. Each
 *              thread requires its own thread structure and the structure is allocated in user memory space (RAM). The
 *              address of the thread’s structure is provided to OS thread-related services.
 *
 *              Thread structure is used as thread ID and a thread is always referenced using this structure.
 * @api
 */
struct nthread {
    struct nthread_stack *      stack;                                          /**<@brief Pointer to top of stack    */
    struct ndlist               array_entry;                                    /**<@brief Priority array entry       */
    uint_fast8_t                priority;                                       /**<@brief Current priority level     */
    uint_fast8_t                opriority;                                      /**<@brief Current priority level     */
    uint_fast8_t                quantum_counter;                                /**<@brief Quantum counter            */
    uint_fast8_t                quantum_reload;                                 /**<@brief Quantum reload value       */
#if   (CONFIG_SEMAPHORE == 1u) || defined(__DOXYGEN__)
    enum nsys_status            op_status;
#endif
#if   (CONFIG_DEBUG_API == 1u) || defined(__DOXYGEN__)
    natomic                     signature;                                      /**<@brief Debug signature            */
#endif
};

/**@brief       Thread type
 * @api
 */
typedef struct nthread nthread;

/** @} *//*-------------------------------------------------------------------------------------------------------*//**
 * @addtogroup  semaphore
 * @{ *//*------------------------------------------------------------------------------------------------------------*/



/**@brief       Semaphore structure
 * @api
 */
struct nsem {
    struct nub_prio_array       prio_array;
    uint32_t                    count;
#if   (CONFIG_DEBUG_API == 1u) || defined(__DOXYGEN__)
    natomic                     signature;                                      /**<@brief Debug signature            */
#endif
};

/**@brief       Semaphore type
 * @api
 */
typedef struct nsem nsem;

/** @} *//*-----------------------------------------------------------------------------------------------------------*/

/*==============================================================================================  GLOBAL VARIABLES  ==*/

/*----------------------------------------------------------------------------------------------------------------*//**
 * @addtogroup  kernel_generic
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

/**@brief       System context
 * @notapi
 */
extern struct nub_sys_ctx global_nub_sys;

/** @} *//*-----------------------------------------------------------------------------------------------------------*/

/*===========================================================================================  FUNCTION PROTOTYPES  ==*/

/*----------------------------------------------------------------------------------------------------------------*//**
 * @addtogroup  kernel_generic
 * @{ *//*------------------------------------------------------------------------------------------------------------*/


/**@brief       Initialize kernel internal data structures
 * @pre         1) `The kernel state == ES_KERN_INACTIVE`, see @ref states.
 * @post        1) `The kernel state == ES_KERN_INIT`.
 * @note        1) This function may be invoked only once.
 * @details     This function must be called first before any other kernel API. It initializes internal data structures
 *              that are used by other kernel functions.
 * @called
 * @fromapp
 * @schedno
 * @api
 */
void nsys_init(
    void);



/**@brief       Start the multi-threading
 * @pre         1) `The kernel state == ES_KERN_INIT`, see @ref states.
 * @post        1) `The kernel state == ES_KERN_RUN`
 * @post        2) The multi-threading execution will commence.
 * @note        1) Once this function is called the execution of threads will start and this function will never return.
 * @details     This function will start multi-threading. Once the multi-threading has started the execution will never
 *              return to this function again (this function never returns).
 * @called
 * @fromapp
 * @schedyes
 * @api
 */
PORT_C_NORETURN void nsys_start(
    void);



/**@brief       Enter Interrupt Service Routine
 * @pre         1) `The kernel state < ES_KERN_INIT`, see @ref states.
 * @note        1) You must call esKernIsrExitI() at the exit of ISR.
 * @note        2) You must invoke nsys_isr_enter_i() and nsys_isr_exit_i() in pair. In other words, for every call to
 *                  nsys_isr_enter_i() at the beginning of the ISR you must have a call to nsys_isr_exit_i() at the end
 *                  of the ISR.
 * @details     Function will notify kernel that you are about to enter interrupt service routine (ISR). This allows
 *              kernel to keep track of interrupt nesting and then only perform rescheduling at the last nested ISR.
 * @called
 * @fromisr
 * @schedno
 * @iclass
 */
void nsys_isr_enter_i(
    void);



/**@brief       Exit Interrupt Service Routine
 * @pre         1) `The kernel state < ES_KERN_INIT`, see @ref states.
 * @note        1) You must invoke nsys_isr_enter_i() and nsys_isr_exit_i() in pair. In other words, for every call to
 *                  nsys_isr_enter_i() at the beginning of the ISR you must have a call to nsys_isr_exit_i() at the end
 *                  of the ISR.
 * @note        2) Rescheduling is prevented when the kernel is locked (see esKernLockEnterI())
 * @details     This function is used to notify kernel that you have completed servicing an interrupt. When the last
 *              nested ISR has completed, the function will call the scheduler to determine whether a new, high-priority
 *              task, is ready to run.
 * @called
 * @fromisr
 * @schedmaybe
 * @iclass
 */
void nsys_isr_exit_i(
    void);



/**@brief       Process the timer event
 * @pre         1) `The kernel state < ES_KERN_INIT`, see @ref states.
 * @details     This function will be called only by port timer interrupt.
 * @notapi
 */
void nub_sys_timer_isr(
    void);



void nsys_lock_enter(
    void);



void nsys_lock_exit(
    void);



void nsys_lock_int_enter(
    nintr_ctx *                 intr_ctx);



void nsys_lock_int_exit(
    nintr_ctx                   intr_ctx);

/** @} *//*-------------------------------------------------------------------------------------------------------*//**
 * @addtogroup  thread
 * @{ *//*------------------------------------------------------------------------------------------------------------*/


/**@brief       Initialize the specified thread
 * @param       thread
 *              Thread: is a pointer to the thread structure, @ref esThread. The structure will be used as information
 *              container for the thread. It is assumed that storage for the `esThread` structure is allocated by the
 *              user code.
 * @param       entry
 *              Function: is a pointer to thread function. Thread function must have the following signature:
 *              `void thread (void * arg)`.
 * @param       arg
 *              Argument: is a void pointer to an optional data area. It's usage is application defined and it is
 *              intended to pass arguments to thread when it is started for the first time.
 * @param       stack
 *              Stack: is a pointer to a allocated memory for thread stack. The pointer always points to the first
 *              element in the array, regardless of what type of stack the CPU is using. The thread's stack is used to
 *              store local variables, function parameters, return addresses. Each thread has its own stack and
 *              different sized stack. The stack type must be an array of @ref esThreadStack.
 * @param       stack_size
 *              Stack Size: specifies the size of allocated stack memory. Size is expressed in bytes. Please see port
 *              documentation about minimal stack size. Usage of C unary operator `sizeof` is the recommended way of
 *              specifying stack size. Another way of specifying required stack size is through the usage of
 *              @ref ES_STACK_SIZE macro.
 * @param       priority
 *              Priority: is the priority of the thread. The higher the number, the higher the priority (the importance)
 *              of the thread. Several threads can have the same priority. Note that lowest (0) and highest
 *              (CFG_SCHED_PRIO_LVL - 1) levels are reserved for kernel threads only.
 * @pre         1) `The kernel state ES_KERN_INACTIVE`, see @ref states.
 * @pre         2) `thread != NULL`
 * @pre         3) `thread->signature != DEF_THD_CONTRACT_SIGNATURE`, the thread structure can't be initialized more
 *                  than once.
 * @pre         4) `fn != NULL`
 * @pre         5) `stack_size >= PORT_STACK_MINSIZE`, see @ref PORT_STACK_MINSIZE.
 * @pre         6) `0 < priority < CFG_SCHED_PRIO_LVL`, see @ref CFG_SCHED_PRIO_LVL.
 * @post        1) `thread->signature == DEF_THD_CONTRACT_SIGNATURE`, each @ref esThread structure will have valid
 *                  signature after initialization.
 * @details     Threads must be created in order for kernel to recognize them as threads. Initialize a thread by calling
 *              esThdInit() and provide arguments specifying to kernel how the thread will be managed. Threads are
 *              always created in the @c ready-to-run state. Threads can be created either prior to the start of
 *              multi-threading (before calling esKernStart()), or by a running thread.
 * @called
 * @fromapp
 * @fromthd
 * @schedmaybe
 * @api
 */
void nthread_init(
    struct nthread *            thread,
    void                     (* entry)(void *),
    void *                      arg,
    struct nthread_stack *      stack,
    size_t                      stack_size,
    uint8_t                     priority);



/**@brief       Terminate the specified thread
 * @pre         1) `The kernel state ES_KERN_INACTIVE`, see @ref states.
 * @pre         2) `thread != NULL`
 * @pre         3) `thread->signature == DEF_THD_CONTRACT_SIGNATURE`, the pointer must point to a valid @ref nthread
 *                  structure.
 * @pre         4) `(thread->esThreadList_.q == NULL) OR (thread->esThreadList_.q == gRdyQueue)`, thread must be either
 *                  in Ready Threads Queue or not be in any run_queue (e.g. not waiting for a synchronization mechanism).
 * @post        1) `thread->signature == ~DEF_THD_CONTRACT_SIGNATURE`,  each @ref esThread structure will have invalid
 *                  signature after termination.
 * @called
 * @fromapp
 * @fromthd
 * @schedmaybe
 * @api
 */
void nthread_term(
    void);



/**@brief       Get the priority of a thread
 * @param       thread
 *              Thread: is pointer to the thread structure, @ref nthread.
 * @return      The priority of the thread pointed by @c thread.
 * @inline
 * @called
 * @fromapp
 * @fromthd
 * @fromisr
 * @schedno
 * @api
 */
static PORT_C_INLINE uint8_t nthread_get_priority(
    const struct nthread *      thread)
{
    return ((uint8_t)thread->priority);
}



/**@brief       Set the priority of the currently running thread
 * @param       priority
 *              Priority: is new priority of the thread pointed by @c thread.
 * @pre         1) `The kernel state < ES_KERN_INACTIVE`, see @ref states.
 * @pre         4) `0 < priority < CONFIG_PRIORITY_LEVELS`, see @ref CONFIG_PRIORITY_LEVELS.
 * @called
 * @fromapp
 * @fromthd
 * @fromisr
 * @schedmaybe
 */
void nthread_set_priority(
    uint8_t                     priority);

/** @} *//*-------------------------------------------------------------------------------------------------------*//**
 * @addtogroup  semaphore
 * @{ *//*------------------------------------------------------------------------------------------------------------*/


/**@brief       Initialize a semaphore
 * @param       sem
 *              Semaphore: points to a semaphore object to initialize.
 * @param       count
 *              Count: is an initial value to set the semaphore to.
 */
void nsem_init(
    struct nsem *               sem,
    uint32_t                    count);



void nsem_term(
    struct nsem *               sem);



/**@brief       Wait on a semaphore
 * @param       sem
 *              Semaphore: points to a semaphore object to initialize.
 */
enum nsys_status nsem_wait(
    struct nsem *               sem);



/**@brief       Wait on a semaphore
 * @param       sem
 *              Semaphore: points to a semaphore object to initialize.
 * @param       time
 *              Time: the timeout time specified in system ticks.
 */


/**@brief       Increment the value of a semaphore
 * @param       sem
 *              Semaphore: points to a semaphore object to initialize.
 */
void nsem_signal(
    struct nsem *               sem);

/** @} *//*---------------------------------------------------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*========================================================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//**@} *//**@} *//***************************************************************************************
 * END of nub.h
 **********************************************************************************************************************/
#endif /* NUB_H_ */
