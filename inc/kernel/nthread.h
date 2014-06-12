/*
 * This file is part of nKernel.
 *
 * Copyright (C) 2010 - 2013 Nenad Radulovic
 *
 * nKernel is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * nKernel is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with nKernel.  If not, see <http://www.gnu.org/licenses/>.
 *
 * web site:    http://github.com/nradulovic
 * e-mail  :    nenad.b.radulovic@gmail.com
 *//***************************************************************************************************************//**
 * @file
 * @author      Nenad Radulovic
 * @brief       Thread header
 * @defgroup    thread Thread
 * @brief       Thread
 *************************************************************************************************************//** @{ */
/**@defgroup    thread_intf Interface
 * @brief       Public interface
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

#ifndef NTHREAD_H_
#define NTHREAD_H_

/*=================================================================================================  INCLUDE FILES  ==*/

#include <stdint.h>
#include <stddef.h>

#include "plat/compiler.h"

#include "kernel/nkernel_config.h"

#include "kernel/nprio_array.h"

/*=======================================================================================================  MACRO's  ==*/

/**@brief       Converts the required stack elements into the stack array index.
 * @param       elem
 *              Number of stack elements: the stack size is expressed in number of elements regardless of the size of
 *              port general purpose registers.
 * @return      Number of stack elements needed for stack usage.
 */
#define NSTACK_SIZE(elem)                   PORT_STACK_SIZE(elem)

/**@brief       Maximum level of priority possible for application thread
 */
#define NTHREAD_PRIORITY_MAX                (CONFIG_PRIORITY_LEVELS - 2u)

/**@brief       Minimum level of priority possible for application thread
 */
#define NTHREAD_PRIORITY_MIN                (1u)

/*----------------------------------------------------------------------------------------------  C++ extern begin  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*====================================================================================================  DATA TYPES  ==*/


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
    struct nprio_array_entry  queue_entry;                                    /**<@brief Priority queue entry       */
    uint_fast8_t                priority;                                       /**<@brief Current priority level     */
    uint_fast8_t                quantum_counter;                                /**<@brief Quantum counter            */
    uint_fast8_t                quantum_reload;                                 /**<@brief Quantum reload value       */
#if   (1u== CONFIG_API_VALIDATION) || defined(__DOXYGEN__)
    natomic                     signature;                                      /**<@brief Debug signature            */
#endif
};

/**@brief       Thread type
 */
typedef struct nthread nthread;

/*==============================================================================================  GLOBAL VARIABLES  ==*/
/*===========================================================================================  FUNCTION PROTOTYPES  ==*/

/**@brief       Initialize the specified thread
 * @param       thread
 *              Thread: is a pointer to the thread structure, @ref esThread. The structure will be used as information
 *              container for the thread. It is assumed that storage for the `esThread` structure is allocated by the
 *              user code.
 * @param       fn
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
 * @pre         6) `0 < priority < CFG_SCHED_PRIO_LVL - 1`, see @ref CFG_SCHED_PRIO_LVL.
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
    void                     (* fn)(void *),
    void *                      arg,
    struct nthread_stack *      stack,
    size_t                      stack_size,
    uint8_t                     priority);

/**@brief       Terminate the specified thread
 * @param       thread
 *              Thread: is a pointer to the thread structure, @ref esThread.
 * @pre         1) `The kernel state ES_KERN_INACTIVE`, see @ref states.
 * @pre         2) `thread != NULL`
 * @pre         3) `thread->signature == DEF_THD_CONTRACT_SIGNATURE`, the pointer must point to a valid @ref nthread
 *                  structure.
 * @pre         4) `(thread->esThreadList_.q == NULL) OR (thread->esThreadList_.q == gRdyQueue)`, thread must be either
 *                  in Ready Threads Queue or not be in any queue (e.g. not waiting for a synchronization mechanism).
 * @post        1) `thread->signature == ~DEF_THD_CONTRACT_SIGNATURE`,  each @ref esThread structure will have invalid
 *                  signature after termination.
 * @called
 * @fromapp
 * @fromthd
 * @schedmaybe
 * @api
 */
void nthread_term(
    struct nthread *            thread);

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
    const struct nthread *      thread) {

    return ((uint8_t)thread->priority);
}

/**@brief       Set the priority of a thread
 * @param       thread
 *              Thread: is pointer to the thread structure, @ref esThread.
 * @param       priority
 *              Priority: is new priority of the thread pointed by @c thread.
 * @pre         1) `The kernel state < ES_KERN_INACTIVE`, see @ref states.
 * @pre         2) `thread != NULL`
 * @pre         3) `thread->signature == DEF_THD_CONTRACT_SIGNATURE`, the pointer must point to a valid @ref nthread
 *                  structure.
 * @pre         4) `0 < priority < CFG_SCHED_PRIO_LVL - 1`, see
 *                  @ref CFG_SCHED_PRIO_LVL.
 * @called
 * @fromapp
 * @fromthd
 * @fromisr
 * @schedmaybe
 * @iclass
 */
void nthread_set_priority_i(
    struct nthread *            thread,
    uint8_t                     priority);

/*------------------------------------------------------------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*========================================================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//**********************************************************************************************
 * END of nthread.h
 **********************************************************************************************************************/
#endif /* NTHREAD_H_ */
