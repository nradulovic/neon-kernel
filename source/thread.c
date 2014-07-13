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
 *//***********************************************************************//**
 * @file
 * @author      Nenad Radulovic
 * @brief       Thread implementation
 * @addtogroup  thread
 *********************************************************************//** @{ */
/**@defgroup    thread_impl Implementation
 * @brief       Implementation
 * @{ *//*--------------------------------------------------------------------*/

/*=========================================================  INCLUDE FILES  ==*/

#include "plat/critical.h"
#include "lib/ndebug.h"
#include "kernel/nthread.h"
#include "kernel/nsched.h"

/*=========================================================  LOCAL MACRO's  ==*/

/**@brief       Thread structure signature.
 * @details     The signature is used to confirm that a structure passed to a
 *              function is indeed a nthread thread structure.
 */
#define THREAD_SIGNATURE                    ((n_native)0xfeedbeeful)

/*======================================================  LOCAL DATA TYPES  ==*/
/*=============================================  LOCAL FUNCTION PROTOTYPES  ==*/
/*=======================================================  LOCAL VARIABLES  ==*/

static const NMODULE_INFO_CREATE("nthread", "Threads", "Nenad Radulovic");

/*======================================================  GLOBAL VARIABLES  ==*/
/*============================================  LOCAL FUNCTION DEFINITIONS  ==*/
/*===================================  GLOBAL PRIVATE FUNCTION DEFINITIONS  ==*/
/*====================================  GLOBAL PUBLIC FUNCTION DEFINITIONS  ==*/

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
    NREQUIRE(NAPI_RANGE,   stack_size >= NCPU_STACK_MINSIZE);
    NREQUIRE(NAPI_RANGE,   priority < CONFIG_PRIORITY_LEVELS);
    NOBLIGATION(thread->signature = THREAD_SIGNATURE);                          /* Validate thread structure          */

    thread->stack           = NCPU_INIT_CTX(stack, stack_size, entry, arg);     /* Make a fake thread stack           */
    nprio_list_init_node(&thread->queue_node, priority);
    thread->opriority       = priority;
    thread->quantum_counter = CONFIG_SCHED_TIME_QUANTUM;
    thread->quantum_reload  = CONFIG_SCHED_TIME_QUANTUM;
    NCRITICAL_LOCK_ENTER(&intr_ctx);
    nsched_insert_i(&thread->queue_node);                                       /* Add to Run Queue                   */
    nsched_reschedule_i();                                                      /* Invoke the scheduler               */
    NCRITICAL_LOCK_EXIT(intr_ctx);

#if   (1u == CONFIG_HOOK_AT_THREAD_INIT)
    hook_at_thread_init(thread);
#endif
}



PORT_C_NORETURN void nthread_term(
    void)
{
    nintr_ctx                   intr_ctx;

#if   (1u == CFG_HOOK_PRE_THD_TERM)
    userPreThdTerm();
#endif
    NCRITICAL_LOCK_ENTER(&intr_ctx);
    nsched_remove_current_i();
    nsched_reschedule_i();
    NCRITICAL_LOCK_EXIT(intr_ctx);

    while (true);                                                               /* Should never come here             */
}



uint8_t nthread_get_priority(
    void)
{
    struct nprio_list_node *    current_node;

    current_node = nsched_get_current();

    return ((uint8_t)nprio_list_priority(current_node));
}



void nthread_set_priority(
    uint8_t                     priority)
{
    nintr_ctx                   intr_ctx;

    NREQUIRE(NAPI_RANGE, priority < CONFIG_PRIORITY_LEVELS);

    NCRITICAL_LOCK_ENTER(&intr_ctx);
    nsched_modify_prio_i(nsched_get_current(), priority);
    nsched_reschedule_i();
    NCRITICAL_LOCK_EXIT(intr_ctx);
}

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//** @} *//*********************************************
 * END of nthread.c
 ******************************************************************************/
