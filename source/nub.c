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
 * @brief       Kernel implementation
 * @addtogroup  kernel
 *********************************************************************//** @{ */
/**@defgroup    kernel_impl Implementation
 * @brief       Implementation
 * @{ *//*--------------------------------------------------------------------*/

/*=========================================================  INCLUDE FILES  ==*/

#include "plat/critical.h"
#include "arch/intr.h"
#include "kernel/nub.h"
#include "kernel/nsched.h"
#include "kernel/nthread.h"

/*=========================================================  LOCAL MACRO's  ==*/

/**@} *//*--------------------------------------------------------------------------------------------------------*//**
 * @name        Timer
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

/**@brief       Timer structure signature.
 * @details     The signature is used to confirm that a structure passed to a timer function is indeed a timer structure.
 */
#define TIMER_SIGNATURE                     ((n_native)0xfeedbef1ul)

/**@} *//*------------------------------------------------------------------------------------------------------------*/

/*======================================================  LOCAL DATA TYPES  ==*/

/*----------------------------------------------------------------------------------------------------------------*//**
 * @name        System timer
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

/**@brief       Main System Timer structure
 * @note        1) Member `ptick` exists only if ADAPTIVE mode is selected. When
 *              this mode is selected then kernel supports more aggressive power
 *              savings.
 */
struct sys_timer
{
    uint_fast16_t       vTmrArmed;                                              /**< @brief The number of armed virtual timers in system.   */
    uint_fast16_t       vTmrPend;                                               /**< @brief The number of pending timers for arming.        */
    //esVTmrTick            ctick;                                                  /**< @brief Current system timer tick value.                */
#if   (1u == CFG_SYSTMR_ADAPTIVE_MODE) || defined(__DOXYGEN__)
    esVTmrTick            ptick;                                                  /**< @brief Pending ticks during the timer sleep mode.      */
#endif
};

/**@} *//*------------------------------------------------------------------------------------------------------------*/


/*=============================================  LOCAL FUNCTION PROTOTYPES  ==*/
/*=======================================================  LOCAL VARIABLES  ==*/
/*======================================================  GLOBAL VARIABLES  ==*/
/*============================================  LOCAL FUNCTION DEFINITIONS  ==*/
/*===================================  GLOBAL PRIVATE FUNCTION DEFINITIONS  ==*/
/*====================================  GLOBAL PUBLIC FUNCTION DEFINITIONS  ==*/

void nsys_init(
    void)
{
#if   (CONFIG_HOOK_SYS_EARLY_INIT == 1)
    hook_at_sys_early_init();
#endif
    NINTR_DISABLE();
    NCPU_MODULE_INIT();
    NCPU_INIT();
    NINTR_MODULE_INIT();
    NINTR_INIT();
    nsched_init();
#if   (CONFIG_HOOK_AT_SYS_LATE_INIT == 1)
    hook_at_sys_late_init();
#endif
}



/* 1)       Since this function will never return it is marked with `noreturn`
 *          attribute to allow for compiler optimizations.
 */
PORT_C_NORETURN void nsys_start(
    void)
{
#if   (CONFIG_HOOK_AT_SYS_START == 1)
    hook_at_sys_start();
#endif
    NCPU_INIT_LATE();
    NINTR_INIT_LATE();
    nsched_start();                                                             /* Start the scheduler.               */

    while (true);                                                               /* Prevent compiler warnings.         */
}



void nsys_timer_isr(
    void) {

    nintr_ctx           intr_ctx;

#if   (1u == CFG_HOOK_PRE_SYSTMR_EVENT)
    userPreSysTmr();
#endif
    NCRITICAL_LOCK_ENTER(&intr_ctx);
    nsched_quantum_i();
    NCRITICAL_LOCK_EXIT(intr_ctx);
}



void nsys_isr_enter_i(
    void)
{
    NCPU_ISR_ENTER();
}



void nsys_isr_exit_i(
    void)
{
    NCPU_ISR_EXIT();

    if (NCPU_IS_ISR_LAST()) {
        nsched_reschedule_isr_i();
    }
}



void nsys_lock_enter(
    void)
{
    nintr_ctx                   intr_ctx;

    NCRITICAL_LOCK_ENTER(&intr_ctx);
    nsched_lock_enter_i();
    NCRITICAL_LOCK_EXIT(intr_ctx);
}



void nsys_lock_exit(
    void)
{
    nintr_ctx                   intr_ctx;

    NCRITICAL_LOCK_ENTER(&intr_ctx);
    nsched_lock_exit_i();
    NCRITICAL_LOCK_EXIT(intr_ctx);
}



void nsys_lock_int_enter(
    nintr_ctx *                 intr_ctx)
{
    NCRITICAL_LOCK_ENTER(intr_ctx);
    nsched_lock_enter_i();
}



void nsys_lock_int_exit(
    nintr_ctx                   intr_ctx)
{
    nsched_lock_exit_i();
    NCRITICAL_LOCK_EXIT(intr_ctx);
}


/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//** @} *//*********************************************
 * END of nub.c
 ******************************************************************************/
