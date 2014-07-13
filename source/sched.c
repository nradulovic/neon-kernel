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
 * @brief       Scheduler implementation
 * @addtogroup  scheduler
 *********************************************************************//** @{ */
/**@defgroup    scheduler_impl Scheduler Implementation
 * @brief       Implementation
 * @{ *//*--------------------------------------------------------------------*/

/*=========================================================  INCLUDE FILES  ==*/

#include "plat/critical.h"
#include "lib/ndebug.h"
#include "kernel/nsched.h"
#include "kernel/nthread.h"

/*=========================================================  LOCAL MACRO's  ==*/
/*======================================================  LOCAL DATA TYPES  ==*/
/*=============================================  LOCAL FUNCTION PROTOTYPES  ==*/

static PORT_C_INLINE_ALWAYS bool sched_is_rescheduled(struct nsched_ctx * ctx);

/*=======================================================  LOCAL VARIABLES  ==*/

static const NMODULE_INFO_CREATE("nsched", "Scheduler", "Nenad Radulovic");

/*======================================================  GLOBAL VARIABLES  ==*/

struct nsched_ctx               g_nsched;

/*============================================  LOCAL FUNCTION DEFINITIONS  ==*/

static PORT_C_INLINE_ALWAYS bool sched_is_rescheduled(struct nsched_ctx * ctx)
{
    struct nthread *        new_thread;
    new_thread = nthread_from_queue_node(nprio_array_peek(&g_nsched.run_queue));/* Get new highest priority thread.   */

    if (new_thread != ctx->current) {                                           /* Context switching is needed only   */
                                                                                /* when current and pending are       */
                                                                                /* different.                         */
        ctx->pending = new_thread;

#if (CONFIG_HOOK_AT_CONTEXT_SWITCH == 1)
        hook_at_context_switch(ctx->current, ctx->pending);
#endif
        return (true);
    } else {
        return (false);
    }
}

/*===================================  GLOBAL PRIVATE FUNCTION DEFINITIONS  ==*/
/*====================================  GLOBAL PUBLIC FUNCTION DEFINITIONS  ==*/

void nsched_init(
    void)
{
    g_nsched.current = NULL;
    g_nsched.pending = NULL;
    g_nsched.lock_count = 0u;
    nprio_array_init(&g_nsched.run_queue);                                      /* Initialize run_queue structure.    */
}



void nsched_start(
    void)
{
    struct nthread *            new_thread;
    nintr_ctx                   intr_ctx;

    NCRITICAL_LOCK_ENTER(&intr_ctx);
    new_thread = nthread_from_queue_node(nprio_array_peek(&g_nsched.run_queue));
                                                                                /* Get the highest priority thread.   */
    g_nsched.pending = new_thread;
    NCRITICAL_LOCK_EXIT(intr_ctx);
    NCPU_DISPATCH_TO_FIRST();                                                   /* Start the first thread.            */
}



struct nprio_list_node * nsched_get_current(
    void)
{
    return (&g_nsched.current->queue_node);
}



void nsched_reschedule_i(
    void)
{
    if (g_nsched.lock_count == 0u) {
        if (sched_is_rescheduled(&g_nsched)) {
            NCPU_DISPATCH();
        }
    }
}



void nsched_reschedule_isr_i(
    void)
{
    if (g_nsched.lock_count == 0u) {
        if (sched_is_rescheduled(&g_nsched)) {
            NCPU_DISPATCH_FROM_ISR();
        }
    }
}



void nsched_lock_enter_i(
    void)
{
    g_nsched.lock_count++;
}



void nsched_lock_exit_i(
    void)
{
    g_nsched.lock_count--;
    nsched_reschedule_i();
}



#if   (1u == CFG_SCHED_POWER_SAVE) || defined(__DOXYGEN__)


void nsched_sleep(
    void)
{
    nintr_ctx           intrCtx;

    NCRITICAL_LOCK_ENTER(&intrCtx);
    nsched_lock_enter_i();
# if (1u == CFG_HOOK_PRE_IDLE)
    userPreIdle();
# endif
# if (1u == CFG_SYSTMR_ADAPTIVE_MODE)
    vTmrImportPendSleepI();                                                     /* Import any pending timers.                               */
    sysTmrDeactivateI();                                                        /* Evaluate timers and set system timer value for wake up.  */
# endif
    do {
# if (0u == CFG_DBG_ENABLE)
        /*
         * TODO: What to do here?
         */
#if 0
    PORT_CRITICAL_EXIT_SLEEP_ENTER();                                           /* Enter sleep state and wait for an interrupt.             */
#endif
# else
    NCRITICAL_LOCK_EXIT(intrCtx);
# endif
    NCRITICAL_LOCK_ENTER(&intrCtx);
    } while (g_nsched.current);
# if (1u == CFG_HOOK_POST_IDLE)
    userPostIdle();
# endif
    nsched_lock_exit_i();
    NCRITICAL_LOCK_EXIT(intrCtx);
}

#endif
#if   (1u == CFG_SCHED_POWER_SAVE) || defined(__DOXYGEN__)


void nsched_wake_up_i(
    void) {

# if (1u == CFG_SYSTMR_ADAPTIVE_MODE)
    sysTmrActivate();                                                           /* Switch to normal system timer operation.                 */
# endif
}

#endif


void nsched_quantum_i(
    void)
{
    if (g_nsched.lock_count == 0u) {
        struct nthread * thread;

        thread = nthread_from_queue_node(nsched_get_current());                 /* Get the current thread             */
        thread->quantum_counter--;                                              /* Decrement current thread time      */
                                                                                /* quantum                            */
        if (thread->quantum_counter == 0u) {
            thread->quantum_counter = thread->quantum_reload;                   /* Reload thread time quantum         */
            nprio_array_rotate(&g_nsched.run_queue, &thread->queue_node);       /* Rotate current thread priority     */
                                                                                /* array level.                       */
        }
    }
}

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//** @} *//*********************************************
 * END of nsched.c
 ******************************************************************************/
