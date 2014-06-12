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
 * @brief       Short desciption of file
 * @addtogroup  module_impl
 *************************************************************************************************************//** @{ */

/*=================================================================================================  INCLUDE FILES  ==*/

#include <stddef.h>

#include "plat/critical.h"
#include "arch/kcore.h"

#include "kernel/nsched.h"
#include "kernel/ndebug.h"
#include "kernel/nprio_array.h"
#include "kernel/nthread.h"

/*=================================================================================================  LOCAL MACRO's  ==*/
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
struct nsched_ctx global_sched_ctx =
{
    NULL,                                                                       /* No thread is currently executing   */
    NULL,                                                                       /* No thread is pending               */
    NSCHED_INACTIVE                                                             /* This is default scheduler state    */
};

/*====================================================================================  LOCAL FUNCTION DEFINITIONS  ==*/
/*===========================================================================  GLOBAL PRIVATE FUNCTION DEFINITIONS  ==*/
/*============================================================================  GLOBAL PUBLIC FUNCTION DEFINITIONS  ==*/

void nsched_init(
    void)
{
    NREQUIRE(NAPI_USAGE,   global_sched_ctx.state == NSCHED_INACTIVE);

    nprio_array_init(&global_run_queue);                                        /* Initialize thread queue structure  */
    global_sched_ctx.state = NSCHED_INIT;
}

void nsched_start(
    void)
{
    struct nthread *            new_thread;
    nintr_ctx                   intr_ctx;

    NREQUIRE(NAPI_USAGE,   global_sched_ctx.state == NSCHED_INIT);              /* Can be called only from            */
                                                                                /* initialization code.               */
    NCRITICAL_LOCK_ENTER(&intr_ctx);
    new_thread = nprio_array_peek(&global_run_queue);                           /* Get the highest priority thread    */
    global_sched_ctx.cthread = new_thread;
    global_sched_ctx.pthread = new_thread;
    global_sched_ctx.state   = NSCHED_RUN;
    NCRITICAL_LOCK_EXIT(intr_ctx);
    NPORT_DISPATCH_TO_FIRST();                                                  /* Start the first thread             */
}

void nsched_register_thread_i(
    struct nthread *            thread)
{
    NREQUIRE(NAPI_USAGE,   global_sched_ctx.state < NSCHED_INACTIVE);
    NREQUIRE(NAPI_POINTER, thread != NULL);

    if (global_sched_ctx.pthread == NULL)                                       /* If this is the first thread created*/
    {
        global_sched_ctx.pthread = thread;                                      /* then make it pending               */
    }
    nsched_add_thread_i(thread);
}

void nsched_add_thread_i(
    struct nthread *            thread)
{
    NREQUIRE(NAPI_USAGE,   global_sched_ctx.state < NSCHED_INACTIVE);
    NREQUIRE(NAPI_POINTER, thread != NULL);

    nprio_array_insert(&global_run_queue, thread);

    if (global_sched_ctx.pthread->priority < thread->priority)
    {
        global_sched_ctx.pthread = thread;
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
        global_sched_ctx.pthread = nprio_array_peek(&global_run_queue);         /* Get new highest priority thread.   */
    }
}

void nsched_evaluate_i(
    void)
{
    global_sched_ctx.pthread = nprio_array_peek(&global_run_queue);             /* Get new highest priority thread.   */
}

void nsched_yield_i(
    void)
{
    NREQUIRE(NAPI_USAGE,   global_sched_ctx.state < NSCHED_INIT);

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
    NREQUIRE(NAPI_USAGE, global_sched_ctx.state < NSCHED_INIT);

    global_sched_ctx.state |= NSCHED_ISR;
}

void nsched_isr_exit_i(
    void)
{
    NREQUIRE(NAPI_USAGE, global_sched_ctx.state < NSCHED_INIT);

    if (global_sched_ctx.state == NSCHED_ISR)
    {
        global_sched_ctx.state &= ~NSCHED_ISR;

        if (global_sched_ctx.cthread != global_sched_ctx.pthread)               /* Context switching is needed only   */
        {                                                                       /* when cthread and pthread are       */
                                                                                /* different.                         */
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

    global_sched_ctx.state |= NSCHED_LOCK;
    global_sched_lock_count++;
}

void nsched_lock_exit_i(
    void)
{
    NREQUIRE(NAPI_USAGE, (global_sched_ctx.state & NSCHED_LOCK) != 0);          /* Scheduler must be in locked state  */

    global_sched_lock_count--;

    if (global_sched_lock_count == 0u)
    {
        global_sched_ctx.state &= ~NSCHED_LOCK;
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
    global_sched_ctx.state = NSCHED_SLEEP;
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

    global_sched_ctx.state = NSCHED_LOCK;
# if (1u == CFG_SYSTMR_ADAPTIVE_MODE)
    sysTmrActivate();                                                           /* Switch to normal system timer operation.                 */
# endif
}
#endif

void sched_quantum_i(
    void)
{
    if (!(global_sched_ctx.state & NSCHED_LOCK))                                /* Round-Robin is not enabled in      */
    {                                                                           /* kernel LOCK state                  */
        struct nthread * thread;

        thread = global_sched_ctx.cthread;                                      /* Get the current thread             */
        thread->quantum_counter--;                                              /* Decrement current thread time      */
                                                                                /* quantum                            */
        if (thread->quantum_counter == 0u)
        {
            thread->quantum_counter = thread->quantum_reload;                   /* Reload thread time quantum         */
            thread = nprio_array_rotate_level(&global_run_queue, thread->priority);     /* Fetch the next thread and  */
                                                                                /* rotate this priority list          */
            if (global_sched_ctx.pthread == global_sched_ctx.cthread)           /* If there is no thread pending      */
            {
                global_sched_ctx.pthread = thread;                              /* Make the new thread pending        */
            }
        }
    }
}

/*========================================================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of nsched.c
 ******************************************************************************/
