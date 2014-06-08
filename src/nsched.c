/*
 * This file is part of esolid-kernel
 *
 * Template version: 1.1.16 (24.12.2013)
 *
 * Copyright (C) 2011, 2012 - Nenad Radulovic
 *
 * esolid-kernel is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * esolid-kernel is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with eSolid.  If not, see <http://www.gnu.org/licenses/>.
 *
 * web site:    http://blueskynet.dyndns-server.com
 * e-mail  :    blueskyniss@gmail.com
 *//***************************************************************************************************************//**
 * @file
 * @author      nenad
 * @brief       Short desciption of file
 * @addtogroup  module_impl
 *************************************************************************************************************//** @{ */

/*=================================================================================================  INCLUDE FILES  ==*/

#include <stddef.h>

#include "plat/critical.h"
#include "arch/kcore.h"

#include "kernel/nsched.h"
#include "kernel/ndebug.h"
#include "kernel/nthread_queue.h"
#include "kernel/nthread.h"

/*=================================================================================================  LOCAL MACRO's  ==*/

/**@brief       Scheduler state variable bit position which defines if the scheduler is locked or not.
 */
#define DEF_SCHED_STATE_LOCK_Msk        (0x01u << 1)


/**@brief       Kernel state variable bit position which defines if the kernel
 *              is in interrupt servicing state.
 */
#define DEF_SCHED_STATE_INTSRV_MSK      (0x01u << 0)

/*==============================================================================================  LOCAL DATA TYPES  ==*/
/*=====================================================================================  LOCAL FUNCTION PROTOTYPES  ==*/
/*===============================================================================================  LOCAL VARIABLES  ==*/

/**@brief       Module identification info
 */
static const NMODULE_INFO_CREATE("Scheduler", "nKernel - RT Kernel", "Nenad Radulovic");

static struct nprio_array global_run_queue;

/**@brief       Kernel Lock Counter
 */
static uint_fast8_t global_sched_lock_count;

/*==============================================================================================  GLOBAL VARIABLES  ==*/

/**@brief       Kernel control initialization
 */
const volatile struct nsched_ctx global_sched_ctx =
{
    NULL,                                                                       /* No thread is currently executing   */
    NULL,                                                                       /* No thread is pending               */
    NSCHED_INACTIVE                                                             /* This is default kernel state       */
};

/*====================================================================================  LOCAL FUNCTION DEFINITIONS  ==*/
/*===========================================================================  GLOBAL PRIVATE FUNCTION DEFINITIONS  ==*/
/*============================================================================  GLOBAL PUBLIC FUNCTION DEFINITIONS  ==*/

void nsched_init(
    void)
{
    nprio_array_init(&global_run_queue);                                                             /* Initialize basic thread queue structure                  */
    ((volatile struct nsched_ctx *)&global_sched_ctx)->state = NSCHED_INIT;
}

void nsched_start(
    void)
{
    struct nthread *            new_thread;
    nintr_ctx                   intr_ctx;

    NCRITICAL_LOCK_ENTER(&intr_ctx);
    new_thread = nprio_array_peek(&global_run_queue);                                                        /* Get the highest priority thread                          */
    ((volatile struct nsched_ctx *)&global_sched_ctx)->cthread = new_thread;
    ((volatile struct nsched_ctx *)&global_sched_ctx)->pthread = new_thread;
    ((volatile struct nsched_ctx *)&global_sched_ctx)->state   = NSCHED_RUN;
    NCRITICAL_LOCK_EXIT(intr_ctx);
    NPORT_DISPATCH_START();                                                        /* Start the first thread                                   */
}

void nsched_init_thread_i(
    struct nthread *            thread)
{
    NREQUIRE(NAPI_USAGE,   global_sched_ctx.state < NSCHED_INACTIVE);
    NREQUIRE(NAPI_POINTER, thread != NULL);

    if (global_sched_ctx.pthread == NULL) {
        ((struct nsched_ctx *)&global_sched_ctx)->pthread = thread;
    }
}

void nsched_add_thread_i(
    struct nthread *            thread)
{
    NREQUIRE(NAPI_USAGE,   global_sched_ctx.state < NSCHED_INACTIVE);
    NREQUIRE(NAPI_POINTER, thread != NULL);

    nprio_array_insert(&global_run_queue, thread);

    if (thread->priority > global_sched_ctx.pthread->priority)
    {
        ((volatile struct nsched_ctx *)&global_sched_ctx)->pthread = thread;
    }
}

/* 1)       If this function is removing currently executed thread or the pending thread then the scheduler will be
 *          invoked to get new highest priority thread.
 */
void nsched_remove_thread_i(
    struct nthread *            thread)
{
    NREQUIRE(NAPI_USAGE,   global_sched_ctx.state < NSCHED_INACTIVE);
    NREQUIRE(NAPI_POINTER, thread != NULL);

    nprio_array_remove(thread);

    if ((global_sched_ctx.cthread == thread) || (global_sched_ctx.pthread == thread))
    {
        ((volatile struct nsched_ctx *)&global_sched_ctx)->pthread = nprio_array_peek(&global_run_queue);                                                         /* Get new highest priority thread.                         */
    }
}

void nsched_evaluate_i(
    void)
{
    ((volatile struct nsched_ctx *)&global_sched_ctx)->pthread = nprio_array_peek(&global_run_queue);                     /* Get new highest priority thread.                         */
}

void nsched_yield_i(
    void)
{
    NREQUIRE(NAPI_USAGE,   global_sched_ctx.state < NSCHED_INACTIVE);

    if (global_sched_ctx.state == NSCHED_RUN)                                   /* Context switching is needed only   */
    {                                                                           /* when cthread and pthread are       */
        if (global_sched_ctx.cthread != global_sched_ctx.pthread)               /* different.                         */
        {
#if   (1u == CFG_HOOK_PRE_CTX_SW)
            userPreCtxSw(global_sched_ctx.cthread, newThd);
#endif
            PORT_DISPATCH();
        }
    }
}

void nsched_isr_enter_i(
    void)
{
    ((volatile struct nsched_ctx *)&global_sched_ctx)->state |= DEF_SCHED_STATE_INTSRV_MSK;
}

void nsched_isr_exit_i(
    void)
{
    NREQUIRE(NAPI_USAGE, NSCHED_INACTIVE > global_sched_ctx.state);

    ((volatile struct nsched_ctx *)&global_sched_ctx)->state &= ~DEF_SCHED_STATE_INTSRV_MSK;

    if (global_sched_ctx.state == NSCHED_ISR)
    {
        if (global_sched_ctx.cthread != global_sched_ctx.pthread)                   /* Context switching is needed only   */
        {                                                                           /* when cthd and pthd are different.  */

#if   (1u == CFG_HOOK_PRE_CTX_SW)
            userPreCtxSw(global_sched_ctx.cthread, global_sched_ctx.pthread);
#endif
            PORT_DISPATCH_ISR();
#if   (1u == CFG_SCHED_POWER_SAVE)
        }
        else if (NSCHED_SLEEP == global_sched_ctx.state)
        {
            sched_wake_up_i();
#endif
        }
    }
}

void nsched_lock_enter_i(
    void)
{
    NREQUIRE(NAPI_USAGE, global_sched_ctx.state < NSCHED_INIT);

    ((volatile struct nsched_ctx *)&global_sched_ctx)->state |= DEF_SCHED_STATE_LOCK_Msk;
    ++global_sched_lock_count;
}

void nsched_lock_exit_i(
    void)
{
    NREQUIRE(NAPI_USAGE, global_sched_ctx.state == NSCHED_LOCK);
    NREQUIRE(NAPI_USAGE, global_sched_lock_count != 0u);

    --global_sched_lock_count;

    if (global_sched_lock_count == 0u)
    {
        ((volatile struct nsched_ctx *)&global_sched_ctx)->state &= ~DEF_SCHED_STATE_LOCK_Msk;
        nsched_yield_i();
    }
}

void nsched_lock_int_enter(
    nintr_ctx *                 lockCtx)
{
    NCRITICAL_LOCK_ENTER(lockCtx);
    nsched_lock_enter_i();
}

void nsched_lock_int_exit(
    nintr_ctx                   lockCtx)
{
    nsched_lock_exit_i();
    NCRITICAL_LOCK_EXIT(lockCtx);
}

#if   (1u == CFG_SCHED_POWER_SAVE) || defined(__DOXYGEN__)
void sched_sleep(
    void) {

    nintr_ctx           intrCtx;

    NCRITICAL_LOCK_ENTER(&intrCtx);
    nsched_lock_enter_i();
# if (1u == CFG_HOOK_PRE_IDLE)
    userPreIdle();
# endif
    ((struct nsched_ctx *)&global_sched_ctx)->state = NSCHED_SLEEP;
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
    } while (global_sched_ctx.cthread == global_sched_ctx.pthread);
# if (1u == CFG_HOOK_POST_IDLE)
    userPostIdle();
# endif
    nsched_lock_exit_i();
    NCRITICAL_LOCK_EXIT(intrCtx);
}
#endif

#if   (1u == CFG_SCHED_POWER_SAVE) || defined(__DOXYGEN__)
void sched_wake_up_i(
    void) {

    ((struct nsched_ctx *)&global_sched_ctx)->state = NSCHED_LOCK;
# if (1u == CFG_SYSTMR_ADAPTIVE_MODE)
    sysTmrActivate();                                                           /* Switch to normal system timer operation.                 */
# endif
}
#endif

void sched_quantum_i(
    void)
{
    if (global_sched_ctx.state < NSCHED_LOCK)
    {                                        /* Round-Robin is not enabled in kernel LOCK state          */
        nthread *       thread;

        thread = global_sched_ctx.cthread;                                                   /* Get the current thread                                   */
        thread->quantum_counter--;                                                       /* Decrement current thread time quantum                    */

        if (thread->quantum_counter == 0u)
        {
            thread->quantum_counter = thread->quantum_reload;                                        /* Reload thread time quantum                               */
            thread = nprio_array_rotate_level(&global_run_queue, thread->priority);/* Fetch the next thread and rotate this priority list     */

            if (global_sched_ctx.cthread == global_sched_ctx.pthread)
            {                                                /* If there is no any other thread pending for switching    */
                ((volatile struct nsched_ctx *)&global_sched_ctx)->pthread = thread;                           /* Make the new thread pending                              */
            }
        }
    }
}

/*========================================================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of nsched.c
 ******************************************************************************/
