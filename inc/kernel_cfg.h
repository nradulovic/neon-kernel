/******************************************************************************
 * This file is part of eSolid-Kernel
 *
 * Copyright (C) 2011, 2012 - Nenad Radulovic
 *
 * eSolid-Kernel is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * eSolid-Kernel is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with eSolid-Kernel; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 *
 * web site:    http://blueskynet.dyndns-server.com
 * e-mail  :    blueskyniss@gmail.com
 *//***********************************************************************//**
 * @file
 * @author  	Nenad Radulovic
 * @brief   	Configuration of Kernel.
 * @addtogroup  kern_cfg
 * @details     Each configuration option or setting has its own default value
 *              when not defined by the application. When application needs to
 *              change a setting it just needs to define a configuration macro
 *              with another value and the default configuration macro will be
 *              overridden.
 *********************************************************************//** @{ */

#ifndef KERNEL_CONFIG_H_
#define KERNEL_CONFIG_H_

/*=========================================================  INCLUDE FILES  ==*/
/*===============================================================  DEFINES  ==*/
/** @cond */

/** @endcond */
/*==============================================================  SETTINGS  ==*/

/*------------------------------------------------------------------------*//**
 * @name        Kernel configuration options and settings
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Enable/disable API arguments validation
 */
#if !defined(CFG_API_VALIDATION)
# define CFG_API_VALIDATION             1U
#endif

/**@brief       Scheduler priority levels
 */
#if !defined(CFG_SCHED_PRIO_LVL)
# define CFG_SCHED_PRIO_LVL             8U
#endif

/**@brief       Scheduler Round-Robin time quantum
 */
#if !defined(CFG_SCHED_TIME_QUANTUM)
# define CFG_SCHED_TIME_QUANTUM         10U
#endif

/**@brief       Scheduler system timer mode
 * @details     Possible values are:
 *              - 0U - fixed mode
 *              - 1U - inhibited mode
 *              - 2U - adaptive mode
 */
#if !defined(CFG_SCHED_SYSTMR_MODE)
# define CFG_SCHED_SYSTMR_MODE          1U
#endif

/**@brief       The frequency of system tick event
 * @note        This setting is valid only if configuration option
 *              @ref CFG_SYSTMR_CLOCK_FREQUENCY is properly set in port
 *              configuration file cpu_cfg.h
 */
#if !defined(CFG_SYSTMR_EVENT_FREQUENCY)
# define CFG_SYSTMR_EVENT_FREQUENCY     100UL
#endif

/**@brief       The size of the system timer counter
 * @details     Possible values are:
 *              - 0U - 8 bit counter
 *              - 1U - 16 bit counter
 *              - 2U - 32 bit counter
 */
#if !defined(CFG_SYSTMR_TICK_TYPE)
# define CFG_SYSTMR_TICK_TYPE           2U
#endif

/** @} *//*---------------------------------------------------------------*//**
 * @name        Kernel hooks
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       System timer event hook function
 */
#if !defined(CFG_HOOK_SYSTMR_EVENT)
# define CFG_HOOK_SYSTMR_EVENT          0U
#endif

/**@brief       Kernel initialization hook function
 */
#if !defined(CFG_HOOK_KERN_INIT)
# define CFG_HOOK_KERN_INIT             0U
#endif

/**@brief       Kernel start hook function
 */
#if !defined(CFG_HOOK_KERN_START)
# define CFG_HOOK_KERN_START            0U
#endif

/**@brief       Thread initialization end hook function
 */
#if !defined(CFG_HOOK_THD_INIT_END)
# define CFG_HOOK_THD_INIT_END          0U
#endif

/**@brief       Thread termination hook function
 */
#if !defined(CFG_HOOK_THD_TERM)
# define CFG_HOOK_THD_TERM              0U
#endif

/**@brief       Context switch hook function
 */
#if !defined(CFG_HOOK_CTX_SW)
# define CFG_HOOK_CTX_SW                0U
#endif

/** @} *//*-------------------------------------------------------------------*/
/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/

#if ((1U != CFG_API_VALIDATION) && (0U != CFG_API_VALIDATION))
# error "eSolid RT Kernel: Configuration option CFG_API_VALIDATION is out of range."
#endif

#if ((2U > CFG_SCHED_PRIO_LVL) || (256U < CFG_SCHED_PRIO_LVL))
# error "eSolid RT Kernel: Configuration option CFG_SCHED_PRIO_LVL is out of range."
#endif

#if ((1U != CFG_HOOK_SYSTMR_EVENT) && (0U != CFG_HOOK_SYSTMR_EVENT))
# error "eSolid RT Kernel: Configuration option CFG_HOOK_SYSTMR_EVENT is out of range."
#endif

#if ((1U != CFG_HOOK_KERN_INIT) && (0U != CFG_HOOK_KERN_INIT))
# error "eSolid RT Kernel: Configuration option CFG_HOOK_KERN_INIT is out of range."
#endif

#if ((1U != CFG_HOOK_KERN_START) && (0U != CFG_HOOK_KERN_START))
# error "eSolid RT Kernel: Configuration option CFG_HOOK_KERN_START is out of range."
#endif

#if ((1U != CFG_HOOK_THD_INIT_END) && (0U != CFG_HOOK_THD_INIT_END))
# error "eSolid RT Kernel: Configuration option CFG_HOOK_THD_INIT_END is out of range."
#endif

#if ((1U != CFG_HOOK_CTX_SW) && (0U != CFG_HOOK_CTX_SW))
# error "eSolid RT Kernel: Configuration option CFG_HOOK_CTX_SW is out of range."
#endif

#if (0 > CFG_SYSTMR_TICK_TYPE) || (2U < CFG_SYSTMR_TICK_TYPE)
# error "eSolid RT Kernel: Configuration option CFG_SYSTMR_TICK_TYPE is out of range."
#endif

#if (2U == CFG_SYSTMR_TICK_TYPE)
typedef uint_fast32_t esTick_T;
#elif (1U == CFG_SYSTMR_TICK_TYPE)
typedef uint_fast16_t esTick_T;
#elif (0U == CFG_SYSTMR_TICK_TYPE)
typedef uint_fast8_t esTick_T;
#endif

/** @endcond *//** @} *//******************************************************
 * END of kernel_config.h
 ******************************************************************************/
#endif /* KERNEL_CONFIG_H_ */
