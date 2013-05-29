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
 * @brief   	Configuration of Kernel - Template.
 * @addtogroup  template_kern_cfg
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
 * @brief       Kernel default configuration
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Enable/disable API arguments validation
 * @details     During the development cycle of the application this option
 *              should be turned on. When this configuration option is turned on
 *              the kernel API functions will also check arguments passed to
 *              them. If an invalid argument is detected the execution of the
 *              application will stop and the user will be informed about the
 *              error condition.
 *
 *              Possible values:
 *              - 0U - API validation is OFF
 *              - 1U - API validation is ON
 *
 * @note        The error checking use userAssert() hook function to provide the
 *              information about the error condition.
 */
#if !defined(CFG_API_VALIDATION)
# define CFG_API_VALIDATION             1U
#endif

/**@brief       Scheduler priority levels
 * @brief       Possible values:
 *              - Min: 2U (two priority levels)
 *              - Max: 256U
 *
 * @warning     Scheduler will have undefined behavior if there is no ready
 *              thread to run (e.g. empty @ref sched_rdyThdQ) at the time it is
 *              invoked.
 */
#if !defined(CFG_SCHED_PRIO_LVL)
# define CFG_SCHED_PRIO_LVL             8U
#endif

/**@brief       Scheduler Round-Robin time quantum
 * @details     This constant is the number of system ticks allowed for the
 *              threads before preemption occurs. Setting this value to zero
 *              disables the preemption for threads with equal priority and the
 *              round robin becomes cooperative. Note that higher priority
 *              threads can still preempt, the kernel is always preemptive.
 * @note        Disabling the round robin preemption makes the kernel more
 *              compact and generally faster.
 */
#if !defined(CFG_SCHED_TIME_QUANTUM)
# define CFG_SCHED_TIME_QUANTUM         10U
#endif

/**@brief       The frequency of system tick event
 * @details     Specify the desired resolution system tick time source. This
 *              setting is valid only if configuration option
 *              @ref CFG_SYSTMR_CLOCK_FREQUENCY is properly set in port
 *              configuration file cpu_cfg.h
 */
#if !defined(CFG_SYSTMR_EVENT_FREQUENCY)
# define CFG_SYSTMR_EVENT_FREQUENCY     100UL
#endif

/** @} *//*---------------------------------------------------------------*//**
 * @name        Kernel hooks
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       System timer event hook function
 * @details     This hook is called just a moment before a system timer event is
 *              processed.
 * @note        This hook will call userSysTmr() function.
 */
#if !defined(CFG_HOOK_SYSTMR_EVENT)
# define CFG_HOOK_SYSTMR_EVENT          0U
#endif

/**@brief       Kernel initialization hook function
 * @details     This hook is called at the beginning of esKernInit() function.
 * @note        This hook will call userKernInit() function.
 */
#if !defined(CFG_HOOK_KERN_INIT)
# define CFG_HOOK_KERN_INIT             0U
#endif

/**@brief       Kernel start hook function
 * @details     This hook is called at the beginning of esKernStart() function.
 * @note        This hook will call userKernStart() function.
 */
#if !defined(CFG_HOOK_KERN_START)
# define CFG_HOOK_KERN_START            0U
#endif

/**@brief       Thread initialization hook function
 * @details     This hook is called at the end of esThdInit() function.
 * @note        This hook will call userThdInitEnd() function.
 */
#if !defined(CFG_HOOK_THD_INIT_END)
# define CFG_HOOK_THD_INIT_END          0U
#endif

/**@brief       Thread termination hook function
 * @details     This hook is called when a thread terminates.
 * @note        This hook will call userThdTerm() function.
 */
#if !defined(CFG_HOOK_THD_TERM)
# define CFG_HOOK_THD_TERM              0U
#endif

/**@brief       Kernel context switch hook function
 * @details     This hook is called at each context switch.
 * @note        This hook will call userCtxSw() function.
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

/** @endcond *//** @} *//******************************************************
 * END of kernel_config.h
 ******************************************************************************/
#endif /* KERNEL_CONFIG_H_ */
