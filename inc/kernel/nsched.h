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

#ifndef NSCHED_H_
#define NSCHED_H_

/*=================================================================================================  INCLUDE FILES  ==*/

#include "plat/compiler.h"
#include "arch/intr.h"

/*=======================================================================================================  MACRO's  ==*/
/*----------------------------------------------------------------------------------------------  C++ extern begin  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*====================================================================================================  DATA TYPES  ==*/

/**@brief       Scheduler state enumeration
 * @details     For more details see: @ref states
 * @api
 */
enum nsched_state
{
    NSCHED_RUN                  = (0x0u << 0),                                  /**<@brief Executing threads          */
    NSCHED_ISR                  = (0x1u << 0),                                  /**<@brief Servicing an interrupt     */
    NSCHED_LOCK                 = (0x1u << 1),                                  /**<@brief Locked state               */
    NSCHED_ISR_LOCK             = (NSCHED_LOCK | NSCHED_ISR),                   /**<@brief Locked while in interrupt  */
    NSCHED_SLEEP                = (0x1u << 2),                                  /**<@brief Sleeping                   */
    NSCHED_INIT                 = (0x1u << 3),                                  /**<@brief Initialization state       */
    NSCHED_INACTIVE             = (0x1u << 4)                                   /**<@brief Scheduler is not active    */
};

/**@brief       Scheduler state type
 * @api
 */
typedef enum nsched_state nsched_state;

/**@brief       Scheduler context structure
 * @details     This structure holds important status data for the scheduler.
 * @notapi
 */
struct nsched_ctx
{
    struct nthread *            cthread;                                                   /**< @brief Pointer to the Current Thread                   */
    struct nthread *            pthread;                                                   /**< @brief Pointer to the Pending Thread to be switched    */
    enum nsched_state           state;                                                  /**< @brief State of kernel                                 */
};

/*======================================================  GLOBAL VARIABLES  ==*/

/**@brief       Kernel control block
 * @note        This variable has Read-Only access rights for application.
 * @notapi
 */
extern const volatile struct nsched_ctx global_sched_ctx;

/*===================================================  FUNCTION PROTOTYPES  ==*/

/**@brief       Initialize Ready Thread Queue structure @ref RdyQueue and Scheduler context structure
 *              @ref global_sched_ctx.
 */
void nsched_init(
    void);

/**@brief       Set the scheduler data structures for multi-threading
 * @details     This function is called just before multi-threading will start.
 */
void nsched_start(
    void);

/**@brief       Initialize scheduler ready structure during the thread add
 *              operation
 * @param       thd
 *              Pointer to the thread currently being initialized.
 * @details     Function will initialize scheduler structures during the init
 *              phase of the kernel.
 */
void nsched_init_thread_i(
    struct nthread *            thread);

/**@brief       Add thread `thread` to the ready thread list and notify the scheduler.
 * @param       thread
 *              Pointer to the initialized thread ID structure, @ref nthread.
 * @pre         1) `The kernel state < ES_KERN_INACTIVE`, see @ref states.
 * @pre         2) `thread != NULL`
 * @pre         4) `thread->esThreadList_.q == NULL`, thread must not be in a queue.
 * @called
 * @fromapp
 * @fromthd
 * @fromisr
 * @schedno
 * @iclass
 */
void nsched_add_thread_i(
    struct nthread *            thread);

/**@brief       Remove thread `thread` from the ready thread list and notify the scheduler.
 * @param       thread
 *              Pointer to the initialized thread ID structure, @ref esThread.
 * @pre         1) `The kernel state < ES_KERN_INACTIVE`, see @ref states.
 * @pre         2) `thread != NULL`
 * @pre         3) `thread->signature == DEF_THD_CONTRACT_SIGNATURE`, the pointer must
 *                  point to a valid @ref esThread structure.
 * @pre         4) `thread->esThreadList_.q == &gRdyQueue`, thread must be in Ready Threads
 *                  queue.
 * @called
 * @fromapp
 * @fromthd
 * @fromisr
 * @schedno
 * @iclass
 */
void nsched_remove_thread_i(
    struct nthread *            thread);

void nsched_evaluate_i(
    void);

/**@brief       Force the scheduler invocation which will evaluate all ready threads and switch to ready thread with the
 *              highest priority
 * @pre         1) `The kernel state < ES_KERN_INACTIVE`, see @ref states.
 * @called
 * @fromthd
 * @schedmaybe
 * @iclass
 */
void nsched_yield_i(
    void);

/**@brief       Force the scheduler invocation which will evaluate all ready threads and switch to ready thread with the
 *              highest priority
 * @pre         1) `The kernel state < ES_KERN_INACTIVE`, see @ref states.
 * @called
 * @fromisr
 * @schedmaybe
 * @iclass
 */
void nsched_yield_isr_i(
    void);

void nsched_isr_enter_i(
    void);

void nsched_isr_exit_i(
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

/**@brief       Enter a critical code lock
 * @param       lock_ctx
 *              Pointer to context variable where to store the current lock context.
 * @called
 * @fromapp
 * @fromthd
 * @schedno
 * @api
 */
void nsched_lock_int_enter(
    nintr_ctx *                 lock_ctx);

/**@brief       Exit a critical code lock
 * @param       lock_ctx
 *              Context variable value
 * @details     Restores the lock context to state before the nsched_lock_int_enter() was called.
 * @called
 * @fromapp
 * @fromthd
 * @schedmaybe
 * @api
 */
void nsched_lock_int_exit(
    nintr_ctx                   lock_ctx);

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
static PORT_C_INLINE struct nthread * nsched_get_current(
    void)
{
    return (global_sched_ctx.cthread);
}

/**@brief       Set the scheduler to sleep
 * @note        This function is used only when @ref CFG_SCHED_POWER_SAVE option
 *              is active.
 */
#if   (1u == CFG_SCHED_POWER_SAVE)
void sched_sleep(
    void);
#endif

/**@brief       Wake up the scheduler
 * @note        This function is used only when @ref CFG_SCHED_POWER_SAVE option
 *              is active.
 */
#if   (1u == CFG_SCHED_POWER_SAVE)
void sched_wake_up_i(
    void);
#endif

/**@brief       Do the Quantum (Round-Robin) scheduling
 */
void sched_quantum_i(
    void);

/*--------------------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of nsched.h
 ******************************************************************************/
#endif /* NSCHED_H_ */
