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
#include "plat/sys_lock.h"
#include "lib/nbias_list.h"
#include "lib/nlist.h"

#include "nkernel_config.h"

/*===============================================================  MACRO's  ==*/

/**@brief       Identifies kernel major version number
 */
#define NSYS_VER_MAJOR                      (1ul)

/**@brief       Identifies kernel minor version number
 */
#define NSYS_VER_MINOR                      (2ul)

/**@brief       Identifies kernel patch level
 */
#define NSYS_VER_PATCH                      (0ul)

/**@brief       Identifies the underlying kernel version number
 */
#define NSYS_VER                                                                \
    (((NSYS_VER_MAJOR) << 24) | (NSYS_VER_MINOR << 16) | (NSYS_VER_PATCH))

/**@brief       Kernel identification string
 */
#define NSYS_ID                             "Neon RT Kernel"

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
    struct nbias_list           queue_node;         /**<@brief Priority queue */
    void                     (* entry)(void *);     /**<@brief Task entry     */
    void *                      stack;              /**<@brief Top of stack   */
    ncpu_reg                    ref;                /**<@brief Reference count*/
#if   (CONFIG_REGISTRY          == 1) || defined(__DOXYGEN__)
    const char *                name;
    struct ndlist               registry_node;
#endif
#if   (CONFIG_DEBUG_API         == 1) || defined(__DOXYGEN__)
    ncpu_reg                    signature;         /**<@brief Debug signature */
#endif
};

/**@brief       Thread type
 * @api
 */
typedef struct nthread nthread;

/*======================================================  GLOBAL VARIABLES  ==*/
/*===================================================  FUNCTION PROTOTYPES  ==*/


void nkernel_init(void);



void nkernel_term(void);



void nkernel_start(void);



void nkernel_isr_enter(void);



void nkernel_isr_exit(void);



void nkernel_schedule_i(
    struct nsys_lock *          lock);



/**@brief       Initialize the specified thread
 * @param       thread
 *              Thread is a pointer to the thread structure, @ref nthread. The
 *              structure will be used as information container for the thread.
 *              It is assumed that storage for the @ref nthread structure is
 *              allocated by the application code.
 * @param       entry
 *              Entry is a pointer to thread entry function. Thread function
 *              must have the following signature: `void thread (void * arg)`.
 * @param       stack
 *              Stack is a pointer to a allocated memory for thread stack. The
 *              pointer always points to the first element in the array,
 *              regardless of what type of stack the CPU is using. The thread's
 *              stack is used to store local variables, function parameters,
 *              return addresses. Each thread has its own stack and different
 *              sized stack. The stack type must be an array of
 *              @ref nthread_stack.
 * @param       priority
 *              Priority is the priority of the thread. The higher the number,
 *              the higher the priority (the importance) of the thread. Several
 *              threads can have the same priority. Note that lowest (`0`) and
 *              highest (`CONFIG_PRIORITY_LEVELS - 1`) levels should be used by
 *              kernel service threads only.
 * @pre         1) `thread != NULL`
 * @pre         2) `thread->signature != THREAD_SIGNATURE`, the thread structure
 *                  must be initialized only once.
 * @pre         3) `entry != NULL`
 * @pre         4) `stack != NULL`
 * @pre         6) `0 < priority < CONFIG_PRIORITY_LEVELS`, see
 *                  @ref CONFIG_PRIORITY_LEVELS.
 * @post        1) `thread->signature = THREAD_SIGNATURE`, each @ref nthread
 *                  structure will have valid signature after initialization.
 * @details     Threads must be created in order for kernel to recognize them as
 *              threads. Initialize a thread by calling nthread_init() and
 *              provide arguments specifying to kernel how the thread will be
 *              managed. Threads are always created in the @c ready-to-run state.
 *              Threads can be created either prior to the start of
 *              multi-threading (before calling nkernel_start()), or by a 
                running thread.
 * @called
 * @fromapp
 * @fromthd
 * @schedmaybe
 * @api
 */
void nthread_init(
    struct nthread *            thread,
    void                     (* entry)(void *),
    void *                      stack,
    uint_fast8_t                priority);



/**@brief       Terminate the current thread
 * @post        1) `thread->signature == ~THREAD_SIGNATURE`,  each @ref nthread
 *                  structure will have invalid signature after termination.
 * @called
 * @fromapp
 * @fromthd
 * @schedyes
 * @api
 */
void nthread_term(void);



struct nthread * nthread_get_current(void);



void nthread_ready_i(
    struct nthread *            thread);



void nthread_block_i(
    struct nthread *            thread);



void nthread_sleep_i(void);



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
uint_fast8_t nthread_get_priority(void);



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
    uint_fast8_t                priority);

/** @} *//*---------------------------------------------------------------*//**
 * @addtogroup  Hook functions
 * @{ *//*--------------------------------------------------------------------*/



extern void hook_on_thread_switch(
    struct nthread *            old_thread,
    struct nthread *            new_thread);

/*--------------------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//** @} *//*********************************************
 * END of nthread.h
 ******************************************************************************/
#endif /* NTHREAD_H */
