/*
 * This file is part of Neon RT Kernel.
 *
 * Copyright (C) 2010 - 2014 Nenad Radulovic
 *
 * Neon RT Kernel is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Neon RT Kernel is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Neon RT Kernel.  If not, see <http://www.gnu.org/licenses/>.
 *
 * web site:    http://github.com/nradulovic
 * e-mail  :    nenad.b.radulovic@gmail.com
 *//***********************************************************************//**
 * @file
 * @author      Nenad Radulovic
 * @brief       Thread header
 * @defgroup    thread Thread
 * @brief       Thread
 *********************************************************************//** @{ */
/**@defgroup    thread_intf Interface
 * @brief       Public interface
 * @details     For more details see @ref threads.
 * @{ *//*--------------------------------------------------------------------*/

#ifndef NTHREAD_H
#define NTHREAD_H

/*=========================================================  INCLUDE FILES  ==*/

#include <stdint.h>
#include <stddef.h>

#include "plat/compiler.h"
#include "kernel/nub_config.h"
#include "lib/nbias_list.h"
#include "lib/nlist.h"
#include "lib/nstatus.h"

/*===============================================================  MACRO's  ==*/

/**@brief       Converts the required stack elements into the stack array index.
 * @param       elem
 *              Number of stack elements: the stack size is expressed in number
 *              of elements regardless of the size of port general purpose
 *              registers.
 * @return      Number of stack elements needed for stack usage.
 * @api
 */
#define NSTACK_SIZE(elem)                   PORT_STACK_SIZE(elem)

/**@brief       Maximum level of priority possible for application thread
 * @api
 */
#define NTHREAD_PRIORITY_MAX                (CONFIG_PRIORITY_LEVELS - 1u)

/**@brief       Minimum level of priority possible for application thread
 * @api
 */
#define NTHREAD_PRIORITY_MIN                (0u)

/*------------------------------------------------------  C++ extern begin  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/

/**@brief       Thread structure
 * @details     A thread structure is a data structure used by kernel to
 *              maintain information about a thread. Each thread requires its
 *              own thread structure and the structure is allocated in user
 *              memory space (RAM). The address of the thread’s structure is
 *              provided to OS thread-related services.
 *
 *              Thread structure is used as thread identification and a thread
 *              is always referenced using this structure.
 * @api
 */
struct nthread
{
    struct nthread_stack *      stack;                                          /**<@brief Pointer to top of stack    */
    struct nbias_list           queue_node;                                     /**<@brief Priority queue node        */
    uint_fast8_t                opriority;                                      /**<@brief Origin priority level      */
    uint_fast8_t                quantum_counter;                                /**<@brief Quantum counter            */
    uint_fast8_t                quantum_reload;                                 /**<@brief Quantum reload value       */
#if   (CONFIG_REGISTRY  == 1u) || defined(__DOXYGEN__)
    const char *                name;
    struct ndlist               registry_node;
#endif
#if   (CONFIG_SEMAPHORE == 1u) || defined(__DOXYGEN__)
    enum n_status               status;
#endif
#if   (CONFIG_DEBUG_API == 1u) || defined(__DOXYGEN__)
    n_native                    signature;                                      /**<@brief Debug signature            */
#endif
};

/**@brief       Thread type
 * @api
 */
typedef struct nthread nthread;

/*======================================================  GLOBAL VARIABLES  ==*/
/*===================================================  FUNCTION PROTOTYPES  ==*/

/**@brief       Initialize the specified thread
 * @param       thread
 *              Thread is a pointer to the thread structure, @ref nthread. The
 *              structure will be used as information container for the thread.
 *              It is assumed that storage for the @ref nthread structure is
 *              allocated by the application code.
 * @param       entry
 *              Entry is a pointer to thread entry function. Thread function
 *              must have the following signature: `void thread (void * arg)`.
 * @param       arg
 *              Arg is a void pointer to an optional thread arguments. It's
 *              usage is application defined and it is intended to pass
 *              arguments to thread entry function when it is started for the
 *              first time. *
 * @param       stack
 *              Stack is a pointer to a allocated memory for thread stack. The
 *              pointer always points to the first element in the array,
 *              regardless of what type of stack the CPU is using. The thread's
 *              stack is used to store local variables, function parameters,
 *              return addresses. Each thread has its own stack and different
 *              sized stack. The stack type must be an array of
 *              @ref nthread_stack.
 * @param       stack_size
 *              Stack size specifies the size of allocated stack memory. Size is
 *              expressed in bytes. Please, see the port documentation about
 *              minimal stack size. Usage of C unary operator `sizeof` is the
 *              recommended way of specifying the stack size. Another way of
 *              specifying required stack size is through the usage of
 *              @ref NSTACK_SIZE macro.
 * @param       priority
 *              Priority is the priority of the thread. The higher the number,
 *              the higher the priority (the importance) of the thread. Several
 *              threads can have the same priority. Note that lowest (`0`) and
 *              highest (`CONFIG_PRIORITY_LEVELS - 1`) levels should be used by
 *              kernel service threads only.
 * @pre         1) `thread != NULL`
 * @pre         2) `thread->signature != THREAD_SIGNATURE`, the thread structure
 *                  can't be initialized more than once.
 * @pre         3) `entry != NULL`
 * @pre         4) `stack != NULL`
 * @pre         5) `stack_size >= NCPU_STACK_MINSIZE`, see
 *                  @ref NCPU_STACK_MINSIZE.
 * @pre         6) `0 < priority < CONFIG_PRIORITY_LEVELS`, see
 *                  @ref CONFIG_PRIORITY_LEVELS.
 * @post        1) `thread->signature = THREAD_SIGNATURE`, each @ref nthread
 *                  structure will have valid signature after initialization.
 * @details     Threads must be created in order for kernel to recognize them as
 *              threads. Initialize a thread by calling nthread_init() and
 *              provide arguments specifying to kernel how the thread will be
 *              managed. Threads are always created in the @c ready-to-run state.
 *              Threads can be created either prior to the start of
 *              multi-threading (before calling nsys_start()), or by a running
 *              thread.
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



/**@brief       Terminate the current thread
 * @post        1) `thread->signature == ~THREAD_SIGNATURE`,  each @ref nthread
 *                  structure will have invalid signature after termination.
 * @called
 * @fromapp
 * @fromthd
 * @schedyes
 * @api
 */
PORT_C_NORETURN void nthread_term(
    void);



/**@brief       Get the priority of the current thread
 * @return      The priority of the current thread.
 * @inline
 * @called
 * @fromapp
 * @fromthd
 * @fromisr
 * @schedno
 * @api
 */
uint8_t nthread_get_priority(
    void);



/**@brief       Set the priority of a thread
 * @param       priority
 *              Priority: is new priority of the current thread.
 * @pre         1) `0 < priority < CONFIG_PRIORITY_LEVELS`, see
 *                  @ref CONFIG_PRIORITY_LEVELS.
 * @called
 * @fromapp
 * @fromthd
 * @fromisr
 * @schedmaybe
 * @iclass
 * @api
 */
void nthread_set_priority(
    uint8_t                     priority);

static PORT_C_INLINE struct nthread * nthread_from_queue_node(
    struct nbias_list *         thread_node)
{
    return (container_of(thread_node, struct nthread, queue_node));
}

/**@brief       Hook function called at thread initialization
 * @details     This function is called only when
 *              @ref CONFIG_HOOK_AT_THREAD_INIT is enabled.
 * @api
 */
extern void hook_at_thread_init(
    struct nthread *            thread);

/*--------------------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//** @} *//*********************************************
 * END of nthread.h
 ******************************************************************************/
#endif /* NTHREAD_H */
