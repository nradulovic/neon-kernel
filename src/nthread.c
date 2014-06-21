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
 * @author      nenad
 * @brief       Short desciption of file
 * @addtogroup  module_impl
 *********************************************************************//** @{ */

/*=================================================================================================  INCLUDE FILES  ==*/

#include "plat/critical.h"
#include "arch/cpu.h"
#include "kernel/ndebug.h"
#include "kernel/nsched.h"
#include "kernel/nthread.h"

/*=================================================================================================  LOCAL MACRO's  ==*/

/**@brief       Thread structure signature.
 * @details     The signature is used to confirm that a structure passed to a function is indeed a nthread
 *              thread structure.
 */
#define THREAD_SIGNATURE                    ((natomic)0xfeedbeeful)

/*==============================================================================================  LOCAL DATA TYPES  ==*/
/*=====================================================================================  LOCAL FUNCTION PROTOTYPES  ==*/
/*===============================================================================================  LOCAL VARIABLES  ==*/
/*==============================================================================================  GLOBAL VARIABLES  ==*/
/*====================================================================================  LOCAL FUNCTION DEFINITIONS  ==*/
/*===========================================================================  GLOBAL PRIVATE FUNCTION DEFINITIONS  ==*/
/*============================================================================  GLOBAL PUBLIC FUNCTION DEFINITIONS  ==*/

void nthread_init(
    struct nthread *            thread,
    void                     (* entry)(void *),
    void *                      arg,
    struct nthread_stack *      stack,
    size_t                      stack_size,
    uint8_t                     priority)
{
    nintr_ctx                   intr_ctx;

    NREQUIRE(NAPI_POINTER, thread != NULL);
    NREQUIRE(NAPI_OBJECT,  thread->signature != THREAD_SIGNATURE);
    NREQUIRE(NAPI_POINTER, entry  != NULL);
    NREQUIRE(NAPI_POINTER, stack  != NULL);
    NREQUIRE(NAPI_RANGE,   stack_size >= PORT_STACK_MINSIZE);
    NREQUIRE(NAPI_RANGE,   priority <= NTHREAD_PRIORITY_MAX);
    NOBLIGATION(thread->signature = THREAD_SIGNATURE);                          /* Validate thread structure          */

    thread->stack           = PORT_CTX_INIT(stack, stack_size, entry, arg);     /* Make a fake thread stack           */
    thread->priority        = priority;
    thread->quantum_counter = CONFIG_SCHED_TIME_QUANTUM;
    thread->quantum_reload  = CONFIG_SCHED_TIME_QUANTUM;
    nprio_array_init_entry(thread);
    NCRITICAL_LOCK_ENTER(&intr_ctx);
    nsched_register_thread_i(thread);                                           /* Register and add to Run Queue      */
    nsched_yield_i();                                                           /* Invoke the scheduler               */
    NCRITICAL_LOCK_EXIT(intr_ctx);

#if   (1u == CFG_HOOK_POST_THD_INIT)
    userPostThdInit();
#endif
}

void nthread_term(
    struct nthread *            thread)
{
    nintr_ctx                   intr_ctx;

    NREQUIRE(NAPI_POINTER, thread != NULL);
    NREQUIRE(NAPI_OBJECT,  thread->signature != THREAD_SIGNATURE);
    NOBLIGATION(thread->signature = ~THREAD_SIGNATURE);                         /* Mark the structure as invalid.     */

#if   (1u == CFG_HOOK_PRE_THD_TERM)
    userPreThdTerm();
#endif
    NCRITICAL_LOCK_ENTER(&intr_ctx);
    nprio_array_remove(&global_sched_ctx.run_queue, thread);
    nsched_yield_i();
    NCRITICAL_LOCK_EXIT(intr_ctx);
}

void nthread_set_priority_i(
    uint8_t                     priority)
{
    NREQUIRE(NAPI_RANGE,   priority <= CONFIG_PRIORITY_LEVELS);

    struct nthread *            thread;

    thread = nsched_get_current();
    nprio_array_remove(&global_sched_ctx.run_queue, thread);
    thread->priority = priority;
    NSCHED_INSERT(thread);
    nsched_evaluate_i();
}

/*========================================================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//**********************************************************************************************
 * END of nthread.c
 **********************************************************************************************************************/
