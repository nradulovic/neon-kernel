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
 *//***********************************************************************//**
 * @file
 * @author  	Nenad Radulovic
 * @brief       Scheduler header
 * @defgroup    scheduler Scheduler
 * @brief       Scheduler
 *********************************************************************//** @{ */
/**@defgroup    scheduler_intf Interface
 * @brief       Public interface
 * @{ *//*--------------------------------------------------------------------*/

#ifndef NSCHED_H
#define NSCHED_H

/*=========================================================  INCLUDE FILES  ==*/

#include <stdint.h>

#include "plat/compiler.h"
#include "kernel/nub_config.h"
#include "lib/nprio_array.h"

/*===============================================================  MACRO's  ==*/
/*------------------------------------------------------  C++ extern begin  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/

/**@brief       Scheduler context structure
 * @details     This structure holds important status data for the scheduler.
 * @notapi
 */
struct nsched_ctx
{
    struct nthread *            current;                                        /**<@brief The current thread         */
    struct nthread *            pending;                                        /**<@brief The pending thread         */
    uint_fast16_t               lock_count;
    struct nprio_array          run_queue;
};

/*======================================================  GLOBAL VARIABLES  ==*/

/**@brief       Kernel control block
 * @notapi
 */
extern struct nsched_ctx g_nsched;

/*===================================================  FUNCTION PROTOTYPES  ==*/

/**@brief       Initialize @ref nsched_ctx structure
 * @api
 */
void nsched_init(
    void);



/**@brief       Set the scheduler data structures for multi-threading
 * @details     This function is called just before multi-threading will start.
 * @api
 */
void nsched_start(
    void);



/**@brief       Get the current thread ID
 * @return      Pointer to current thread ID structure @ref nthread.
 * @inline
 * @called
 * @fromapp
 * @fromthd
 * @fromisr
 * @schedno
 * @api
 */
struct nprio_list_node * nsched_get_current(
    void);



/**@brief       Add thread `thread` to the ready thread list and notify the
 *              scheduler.
 * @param       thread
 *              Pointer to the initialized thread ID structure, @ref nthread.
 * @pre         1) `The kernel state < ES_KERN_INACTIVE`, see @ref states.
 * @pre         2) `thread != NULL`
 * @pre         3) `thread->esThreadList_.q == NULL`, thread must not be in a
 *                  queue.
 * @called
 * @fromapp
 * @fromthd
 * @fromisr
 * @schedno
 * @iclass
 */
static PORT_C_INLINE void nsched_insert_i(
    struct nprio_list_node *    thread_node)
{
    nprio_array_insert(&g_nsched.run_queue, thread_node);
}



/**@brief       Remove thread `thread` from the ready thread list and notify the
 *              scheduler.
 * @param       thread
 *              Pointer to the initialized thread ID structure, @ref nthread.
 * @pre         1) `The kernel state < ES_KERN_INACTIVE`, see @ref states.
 * @pre         2) `thread != NULL`
 * @pre         3) `thread->signature == DEF_THD_CONTRACT_SIGNATURE`, the
 *                  pointer must point to a valid @ref nthread structure.
 * @pre         4) `thread->esThreadList_.q == &gRdyQueue`, thread must be in
 *                  run queue.
 * @called
 * @fromapp
 * @fromthd
 * @fromisr
 * @schedno
 * @iclass
 */
static PORT_C_INLINE void nsched_remove_i(
    struct nprio_list_node *    thread_node)
{
    nprio_array_remove(&g_nsched.run_queue, thread_node);
}



static PORT_C_INLINE struct nprio_list_node * nsched_remove_current_i(
    void)
{
    struct nprio_list_node *    thread_node;

    thread_node = nsched_get_current();
    nsched_remove_i(thread_node);

    return (thread_node);
}



/**@brief       Modify the specified thread priority.
 */
static PORT_C_INLINE void nsched_modify_prio_i(
    struct nprio_list_node *    thread_node,
    uint_fast8_t                priority)
{
    nprio_array_remove(&g_nsched.run_queue, thread_node);
    nprio_list_set_priority(thread_node, priority);
    nprio_array_insert(&g_nsched.run_queue, thread_node);
}



/**@brief       Force the scheduler invocation which will evaluate all ready
 *                  threads and switch to ready thread with the
 *              highest priority
 * @pre         1) `The kernel state < ES_KERN_INACTIVE`, see @ref states.
 * @called
 * @fromthd
 * @schedmaybe
 * @iclass
 */
void nsched_reschedule_i(
    void);



/**@brief       Force the scheduler invocation which will evaluate all ready
 *              threads and switch to ready thread with the highest priority
 * @pre         1) `The kernel state < ES_KERN_INACTIVE`, see @ref states.
 * @called
 * @fromisr
 * @schedmaybe
 * @iclass
 */
void nsched_reschedule_isr_i(
    void);



/**@brief       Lock the scheduler
 * @pre         1) `The kernel state < ES_KERN_INIT`, see @ref states.
 * @called
 * @fromthd
 * @schedno
 * @iclass
 */
void nsched_lock_enter_i(
    void);



/**@brief       Unlock the scheduler
 * @pre         1) `The kernel state < ES_KERN_INIT`, see @ref states.
 * @pre         2) `gKernLockCnt > 0u`, current number of locks must be greater
 *                  than zero, in other words: each call to kernel lock function
 *                  must have its matching call to kernel unlock function.
 * @called
 * @fromthd
 * @schedmaybe
 * @iclass
 */
void nsched_lock_exit_i(
    void);



/**@brief       Set the scheduler to sleep
 * @note        This function is used only when @ref CFG_SCHED_POWER_SAVE option
 *              is active.
 */
#if   (1u == CFG_SCHED_POWER_SAVE)
void nsched_sleep(
    void);
#else
# define sched_sleep()                      (void)0
#endif



/**@brief       Wake up the scheduler
 * @note        This function is used only when @ref CFG_SCHED_POWER_SAVE option
 *              is active.
 */
#if   (1u == CFG_SCHED_POWER_SAVE)
void nsched_wake_up_i(
    void);
#else
# define sched_wake_up_i()                  (void)0
#endif



/**@brief       Do the Quantum (Round-Robin) scheduling
 */
void nsched_quantum_i(
    void);


extern void hook_at_context_switch(
    struct nthread *            old_thread,
    struct nthread *            new_thread);

/*--------------------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//** @} *//*********************************************
 * END of nsched.h
 ******************************************************************************/
#endif /* NSCHED_H */
