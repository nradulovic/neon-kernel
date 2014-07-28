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
 *//***************************************************************************************************************//**
 * @file
 * @author  	Nenad Radulovic
 * @brief       Interface of ARM Cortex-M3 System Timer port.
 * @addtogroup  arm-none-eabi-gcc-v7-m
 *********************************************************************//** @{ */

#ifndef ES_ARCH_SYSTIMER_H_
#define ES_ARCH_SYSTIMER_H_

/*=========================================================  INCLUDE FILES  ==*/

#include <stdint.h>

#include "plat/compiler.h"
#include "arch/systimer_config.h"
#include "arch/cortex_m3.h"
#include "family/profile.h"

/*===============================================================  MACRO's  ==*/

/*------------------------------------------------------------------------*//**
 * @name        Port constants
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       System timer one tick value
 */
#define ES_SYSTIMER_ONE_TICK                                                    \
    (CONFIG_SYSTIMER_CLOCK_FREQ / CONFIG_SYSTIMER_EVENT_FREQ)

/**@brief       Maximum number of ticks without overflowing the system timer
 */
#define ES_SYSTIMER_MAX_TICKS                                                   \
    (ES_PROFILE_MAX_SYSTIMER_VAL / ES_SYSTIMER_ONE_TICK)

/** @} *//*---------------------------------------------  C++ extern base  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/

/**@brief       System timer hardware register type.
 */
typedef unsigned int nsys_timer_tick;

/*======================================================  GLOBAL VARIABLES  ==*/
/*===================================================  FUNCTION PROTOTYPES  ==*/

/*------------------------------------------------------------------------*//**
 * @name        System timer management
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Initialize and start the system timer
 * @inline
 */
static PORT_C_INLINE_ALWAYS void portSysTimerInit_(
    nsys_timer_tick      val) {

    PORT_SYSTICK->CTRL &= ~PORT_SYSTICK_CTRL_ENABLE_Msk;                        /* Disable SysTick Timer                                    */
    PORT_SYSTICK->LOAD  = val - 1u;                                             /* Set SysTick reload register                              */
    PORT_SYSTICK->VAL   = 0u;
    PORT_SYSTICK->CTRL  = PORT_SYSTICK_CTRL_ENABLE_Msk;                         /* SysTick uses the processor clock.                        */
}

/**@brief       Stop and terminate the system timer
 * @inline
 */
static PORT_C_INLINE_ALWAYS void portSysTimerTerm_(
    void) {

    PORT_SYSTICK->CTRL &= ~PORT_SYSTICK_CTRL_ENABLE_Msk;                        /* Disable SysTick Timer                                    */
    PORT_SYSTICK->LOAD  = 0u;                                                   /* Set SysTick reload register                              */
    PORT_SYSTICK->VAL   = 0u;
}

/**@brief       Get free counter value
 * @inline
 */
static PORT_C_INLINE_ALWAYS nsys_timer_tick portSysTimerGetCVal_(
    void) {

    return (PORT_SYSTICK->VAL);
}

/**@brief       Get reload counter value
 * @inline
 */
static PORT_C_INLINE_ALWAYS nsys_timer_tick portSysTimerGetRVal_(
    void) {

    return (PORT_SYSTICK->LOAD);
}

/**@brief       Load the system timer Reload value register
 * @inline
 */
static PORT_C_INLINE_ALWAYS void portSysTimerReload_(
    nsys_timer_tick      val) {

    --val;
    PORT_SYSTICK->CTRL &= ~PORT_SYSTICK_CTRL_ENABLE_Msk;
    PORT_SYSTICK->LOAD  = val;
    PORT_SYSTICK->VAL   = 0u;
    PORT_SYSTICK->CTRL |= PORT_SYSTICK_CTRL_ENABLE_Msk;
}

/**@brief       Enable the system timer
 * @inline
 */
static PORT_C_INLINE_ALWAYS void portSysTimerEnable_(
    void) {

    PORT_SYSTICK->CTRL |= PORT_SYSTICK_CTRL_ENABLE_Msk;
}

/**@brief       Disable the system timer
 * @inline
 */
static PORT_C_INLINE_ALWAYS void ntimer_disable(
    void) {

    PORT_SYSTICK->CTRL &= ~PORT_SYSTICK_CTRL_ENABLE_Msk;
}

/**@brief       Disable the system timer interrupt
 * @inline
 */
static PORT_C_INLINE_ALWAYS void portSysTimerIsrEnable_(
    void) {

    PORT_SYSTICK->CTRL |= PORT_SYSTICK_CTRL_TICKINT_Msk;
}

/**@brief       Enable the system timer interrupt
 * @inline
 */
static PORT_C_INLINE_ALWAYS void portSysTimerIsrDisable_(
    void) {

    PORT_SCB->ICSR |= PORT_SCB_ICSR_PENDSTCLR_Msk;
    PORT_SYSTICK->CTRL &= ~PORT_SYSTICK_CTRL_TICKINT_Msk;
}

void ntimer_module_init(
    void);

void ntimer_module_term(
    void);

void ntimer_set_handler(
    void             (* handler)(void),
    uint_fast8_t        level);

void portSysTimerHandler(
    void);

/** @} *//*-----------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of systimer.h
 ******************************************************************************/
#endif /* ES_ARCH_SYSTIMER_H_ */
