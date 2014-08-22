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
 * @author  	Nenad Radulovic
 * @brief       Port core module header
 * @addtogroup  arm-none-eabi-gcc
 *********************************************************************//** @{ */
/**@defgroup    arm-none-eabi-gcc-v7-m-core ARM Cortex M3/M4 Core module
 * @brief       Port core module
 * @{ *//*--------------------------------------------------------------------*/

#ifndef NPORT_CORE_H
#define NPORT_CORE_H

/*=========================================================  INCLUDE FILES  ==*/

#include <stdint.h>
#include <stdbool.h>

#include "plat/compiler.h"
#include "family/profile.h"
#include "arch/ncore_config.h"

#include "cortex_m3.h"

/*===============================================================  MACRO's  ==*/

/*------------------------------------------------------------------------*//**
 * @name        CPU management macros
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Specifies bit-width of general purpose registers
 */
#define NCPU_DATA_WIDTH                     32u

/**@brief       Specifies data alignment for optimal performance
 */
#define NCPU_DATA_ALIGNMENT                 4u

#define NCPU_REG_MAX                        UINT32_MAX

/**@} *//*----------------------------------------------------------------*//**
 * @name        Interrupt service management macros
 * @{ *//*--------------------------------------------------------------------*/

#define NISR_PRIO_TO_CODE(prio)                                                 \
    (((prio) << (8u - PORT_ISR_PRIO_BITS)) & 0xfful)

#define NISR_CODE_TO_PRIO(code)                                                 \
    (((code) & 0xfful) >> (8u - PORT_ISR_PRIO_BITS))

#define nisr_exit()                         (void)0

/**@} *//*----------------------------------------------------------------*//**
 * @name        Core timer macros
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Core timer one tick value
 */
#define NCORE_TIMER_ONE_TICK                                                    \
    (CONFIG_SYSTIMER_CLOCK_FREQ / CONFIG_SYSTIMER_EVENT_FREQ)

/**@brief       Maximum number of ticks without overflowing the core timer
 */
#define NCORE_TIMER_MAX_TICKS                                                   \
    (NPROFILE_MAX_SYSTIMER_VAL / NCORE_TIMER_ONE_TICK)

/**@} *//*----------------------------------------------  C++ extern base  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/

/**@brief General purpose registers are 32bit wide.
 */
typedef unsigned int ncpu_reg;

/**@brief       Interrupt context type
 * @details     This type is used to declare variable type which will hold
 *              interrupt context data.
 */
typedef unsigned int nisr_ctx;

/**@brief       Core timer hardware register type.
 */
typedef unsigned int ncore_timer_tick;

/*======================================================  GLOBAL VARIABLES  ==*/
/*===================================================  FUNCTION PROTOTYPES  ==*/

/*------------------------------------------------------------------------*//**
 * @name        CPU arithmetic/logic operations
 * @{ *//*--------------------------------------------------------------------*/


/**@brief       Stop the further CPU execution
 */
PORT_C_INLINE
void ncpu_stop(void)
{
    while (true) {
    __asm__ __volatile__ (
        "@  ncpu_stop                                       \n"
        "   wfe                                             \n");
    }
}



/**@brief       Computes integer logarithm base 2
 */
PORT_C_INLINE_ALWAYS
uint_fast8_t ncpu_log2(
    ncpu_reg                    value)
{
    uint_fast8_t                clz;

    __asm__ __volatile__ (
        "@  ncpu_log2                                       \n"
        "   clz    %0, %1                                   \n"
        : "=r"(clz)
        : "r"(value));

    return (31u - clz);
}



/**@brief       Computes integer exponent base 2
 */
PORT_C_INLINE_ALWAYS
ncpu_reg ncpu_exp2(
    uint_fast8_t                value)
{
    return (0x1u << value);
}



PORT_C_INLINE_ALWAYS
void ncpu_sat_increment(
    ncpu_reg *                  value)
{
    if (*value != NCPU_REG_MAX) {
        (*value)++;
    }
}



PORT_C_INLINE_ALWAYS
void ncpu_sat_decrement(
    ncpu_reg *                  value)
{
    if (*value != 0u) {
        (*value)--;
    }
}

/**@} *//*----------------------------------------------------------------*//**
 * @name        Interrupt management
 * @{ *//*--------------------------------------------------------------------*/


/**@brief       Enable all interrupts
 */
PORT_C_INLINE
void nisr_enable(void)
{
    __asm __volatile__ (
        "@  nisr_enable                                     \n"
        "   cpsie   i                                       \n");
}



/**@brief       Disable all interrupts
 */
PORT_C_INLINE
void nisr_disable(void)
{
    __asm __volatile__ (
        "@  nisr_disable                                    \n"
        "   cpsid   i                                       \n");
}



/**@brief       Set the new interrupt priority state
 * @param       new_mask
 *              New interrupt priority mask or new state of interrupts
 * @note        Depending on @ref CONFIG_ISR_MAX_PRIO setting this function will
 *              either set the new priority of allowed interrupts or just
 *              disable/enable all interrupts.
 */
PORT_C_INLINE
void nisr_set_mask(
    nisr_ctx                   new_mask)
{
#if (CONFIG_ISR_MAX_PRIO != 0)
    __asm __volatile__ (
        "@  nisr_set_mask                                   \n"
        "   msr    basepri, %0                              \n"
        :
        : "r"(new_mask));
#else
    __asm __volatile__ (
        "@  nisr_set_mask                                   \n"
        "   msr    primask, %0                              \n"
        :
        : "r"(new_mask));
#endif
}



/**@brief       Get old and set new interrupt priority mask
 * @return      Current interrupt priority mask
 */
PORT_C_INLINE
nisr_ctx nisr_replace_mask(
    nisr_ctx                   new_mask)
{
    nisr_ctx                   old_mask;

#if (CONFIG_ISR_MAX_PRIO != 0)
    __asm __volatile__ (
        "@  nisr_replace_mask                               \n"
        "   mrs     %0, basepri                             \n"
        "   msr     basepri, %1                             \n"
        : "=&r"(old_mask)
        : "r"(new_mask));
#else
    __asm __volatile__ (
        "@  nisr_replace_mask                               \n"
        "   mrs     %0, primask                             \n"
        "   msr    primask, %1                              \n"
        : "=&r"(old_mask)
        : "r"(new_mask));
#endif

    return (old_mask);
}



PORT_C_INLINE
void nisr_enter(void)
{
    extern bool g_isr_is_active;
    
    g_isr_is_active = true;
}



PORT_C_INLINE
void nisr_pend_kernel(void)
{
    PORT_SCB->ICSR |= PORT_SCB_ICSR_PENDSVSET_Msk;
}



PORT_C_INLINE
bool nisr_is_active(void)
{
    extern bool g_isr_is_active;
    
    return (g_isr_is_active);
}

/**@} *//*----------------------------------------------------------------*//**
 * @name        Core timer management
 * @{ *//*--------------------------------------------------------------------*/


/**@brief       Initialize and start the system timer
 */
PORT_C_INLINE
void ncore_timer_init(
    ncore_timer_tick            val)
{
    PORT_SYSTICK->CTRL &= ~PORT_SYSTICK_CTRL_ENABLE_Msk;     /* Disable timer */
    PORT_SYSTICK->LOAD  = val - 1u;                    /* Set reload register */
    PORT_SYSTICK->VAL   = 0u;
    PORT_SYSTICK->CTRL  = PORT_SYSTICK_CTRL_ENABLE_Msk;  /* Use the CPU clock */
}



/**@brief       Stop and terminate the system timer
 */
PORT_C_INLINE
void ncore_timer_term(void)
{
    PORT_SYSTICK->CTRL &= ~PORT_SYSTICK_CTRL_ENABLE_Msk;
}



/**@brief       Get free counter value
 */
PORT_C_INLINE
ncore_timer_tick ncore_timer_get_current(void)
{
    return (PORT_SYSTICK->VAL);
}



/**@brief       Get reload counter value
 */
PORT_C_INLINE
ncore_timer_tick ncore_timer_get_reload(void)
{
    return (PORT_SYSTICK->LOAD);
}



/**@brief       Load the system timer Reload value register
 */
PORT_C_INLINE
void ncore_timer_load(
    ncore_timer_tick            val)
{
    --val;
    PORT_SYSTICK->CTRL &= ~PORT_SYSTICK_CTRL_ENABLE_Msk;
    PORT_SYSTICK->LOAD  = val;
    PORT_SYSTICK->VAL   = 0u;
    PORT_SYSTICK->CTRL |= PORT_SYSTICK_CTRL_ENABLE_Msk;
}



/**@brief       Enable the system timer
 */
PORT_C_INLINE
void ncore_timer_enable(void)
{
    PORT_SYSTICK->CTRL |= PORT_SYSTICK_CTRL_ENABLE_Msk;
}



/**@brief       Disable the system timer
 */
PORT_C_INLINE
void ncore_timer_disable(void)
{
    PORT_SYSTICK->CTRL &= ~PORT_SYSTICK_CTRL_ENABLE_Msk;
}



/**@brief       Disable the system timer interrupt
 */
PORT_C_INLINE
void ncore_timer_isr_enable(void)
{
    PORT_SYSTICK->CTRL |= PORT_SYSTICK_CTRL_TICKINT_Msk;
}




/**@brief       Enable the system timer interrupt
 */
PORT_C_INLINE
void ncore_timer_isr_disable(void)
{
    PORT_SCB->ICSR     |= PORT_SCB_ICSR_PENDSTCLR_Msk;
    PORT_SYSTICK->CTRL &= ~PORT_SYSTICK_CTRL_TICKINT_Msk;
}



/**@brief       Register a handler function to core timer
 * @param       handler
 *              Handler callback function
 * @param       slot
 *              Occupy a predefined slot
 */
void ntimer_set_handler(
    void                     (* handler)(void),
    uint_fast8_t                slot);


/**@} *//*----------------------------------------------------------------*//**
 * @name        Generic port functions
 * @{ *//*--------------------------------------------------------------------*/


/**@brief       Initialize port
 */
void ncore_init(void);



/**@brief       Terminate port
 */
void ncore_term(void);



extern void ncore_timer_isr(void);



extern void ncore_kernel_isr(void);

/** @} *//*-----------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//** @} *//*********************************************
 * END of nport_core.h
 ******************************************************************************/
#endif /* NPORT_CORE_H */
