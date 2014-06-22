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
 *//***************************************************************************************************************//**
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

#if !defined(KERNEL_CONFIG_H_)
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

/**@brief       Scheduler priority levels
 * @details     The number of priority levels. Each priority level can have
 *              several threads. Possible values:
 *              - Min: 3U (three priority levels)
 *              - Max: 256U
 */
#if !defined(CONFIG_PRIORITY_LEVELS)
# define CONFIG_PRIORITY_LEVELS             8U
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

/**@brief       Enable/disable scheduler power savings mode
 * @details     Possible values are:
 *              - 0U - power saving is disabled
 *              - 1U - power saving is enabled
 */
#if !defined(CFG_SCHED_POWER_SAVE)
# define CFG_SCHED_POWER_SAVE           0U
#endif

/**@brief       System timer mode
 * @details     Possible values are:
 *              - 0U - adaptive mode is disabled
 *              - 1U - adaptive mode is enabled
 */
#if !defined(CFG_SYSTMR_MODE)
# define CFG_SYSTMR_ADAPTIVE_MODE       0U
#endif

/**@brief       The frequency of system tick event
 * @details     Specify the desired resolution system tick time source. This
 *              setting is valid only if configuration option
 *              `PORT_CFG_SYSTMR_CLOCK_FREQ` is properly set in port system
 *              control configuration file sysctrl_cfg.h
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
 * @details     This hook is called just a moment before a system timer event is
 *              processed.
 * @note        This hook will call userPreSysTmr() function.
 */
#if !defined(CFG_HOOK_PRE_SYSTMR_EVENT)
# define CFG_HOOK_PRE_SYSTMR_EVENT      0U
#endif

/**@brief       Pre kernel initialization hook function
 * @details     This hook is called at the beginning of esKernInit() function.
 * @note        This hook will call userPreKernInit() function.
 */
#if !defined(CFG_HOOK_PRE_KERN_INIT)
# define CFG_HOOK_PRE_KERN_INIT         0U
#endif

/**@brief       Post kernel initialization hook function
 * @note        This hook will call userPostKernInit() function.
 */
#if !defined(CFG_HOOK_PORT_KERN_INIT)
# define CFG_HOOK_POST_KERN_INIT        0U
#endif

/**@brief       Pre kernel start hook function
 * @details     This hook is called at the beginning of esKernStart() function.
 * @note        This hook will call userPreKernStart() function.
 */
#if !defined(CFG_HOOK_PRE_KERN_START)
# define CFG_HOOK_PRE_KERN_START        0U
#endif

/**@brief       Post thread initialization hook function
 * @details     This hook is called at the end of esThdInit() function.
 * @note        This hook will call userPostThdInit() function.
 */
#if !defined(CFG_HOOK_POST_THD_INIT)
# define CFG_HOOK_POST_THD_INIT         0U
#endif

/**@brief       Pre thread termination hook function
 * @details     This hook is called when a thread terminates.
 * @note        This hook will call userPreThdTerm() function.
 */
#if !defined(CFG_HOOK_PRE_THD_TERM)
# define CFG_HOOK_PRE_THD_TERM          0U
#endif

/**@brief       Pre idle hook function
 * @note        This hook will call userPreIdle() function.
 */
#if !defined(CFG_HOOK_PRE_IDLE)
# define CFG_HOOK_PRE_IDLE              0U
#endif

/**@brief       Post idle hook function
 * @note        This hook will call userPostIdle() function.
 */
#if !defined(CFG_HOOK_POST_IDLE)
# define CFG_HOOK_POST_IDLE             0U
#endif

/**@brief       Pre context switch hook function
 * @details     This hook is called before each context switch.
 * @note        This hook will call userPreCtxSw() function.
 */
#if !defined(CFG_HOOK_PRE_CTX_SW)
# define CFG_HOOK_PRE_CTX_SW            0U
#endif

/** @} *//*-------------------------------------------------------------------*/
/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/

#if ((3U > CONFIG_PRIORITY_LEVELS) || (256U < CONFIG_PRIORITY_LEVELS))
# error "NUB RT Kernel RT Kernel: Configuration option CFG_SCHED_PRIO_LVL is out of range."
#endif

#if ((1U != CFG_SCHED_POWER_SAVE) && (0U != CFG_SCHED_POWER_SAVE))
# error "NUB RT Kernel RT Kernel: Configuration option CFG_SCHED_POWER_SAVE is out of range."
#endif

#if ((1U != CFG_SYSTMR_ADAPTIVE_MODE) && (0U != CFG_SYSTMR_ADAPTIVE_MODE))
# error "NUB RT Kernel RT Kernel: Configuration option CFG_SYSTMR_ADAPTIVE_MODE is out of range."
#endif

#if ((0U == CFG_SCHED_POWER_SAVE) && (1U == CFG_SYSTMR_ADAPTIVE_MODE))
# error "NUB RT Kernel RT Kernel: Configuration option CFG_SCHED_PRIO_LVL must be enabled when CFG_SYSTMR_ADAPTIVE_MODE is enabled, too."
#endif

#if ((1U != CFG_HOOK_PRE_SYSTMR_EVENT) && (0U != CFG_HOOK_PRE_SYSTMR_EVENT))
# error "NUB RT Kernel RT Kernel: Configuration option CFG_HOOK_PRE_SYSTMR_EVENT is out of range."
#endif

#if ((1U != CFG_HOOK_PRE_KERN_INIT) && (0U != CFG_HOOK_PRE_KERN_INIT))
# error "NUB RT Kernel RT Kernel: Configuration option CFG_HOOK_PRE_KERN_INIT is out of range."
#endif

#if ((1U != CFG_HOOK_POST_KERN_INIT) && (0U != CFG_HOOK_POST_KERN_INIT))
# error "NUB RT Kernel RT Kernel: Configuration option CFG_HOOK_POST_KERN_INIT is out of range."
#endif

#if ((1U != CFG_HOOK_PRE_KERN_START) && (0U != CFG_HOOK_PRE_KERN_START))
# error "NUB RT Kernel RT Kernel: Configuration option CFG_HOOK_PRE_KERN_START is out of range."
#endif

#if ((1U != CFG_HOOK_POST_THD_INIT) && (0U != CFG_HOOK_POST_THD_INIT))
# error "NUB RT Kernel RT Kernel: Configuration option CFG_HOOK_POST_THD_INIT is out of range."
#endif

#if ((1U != CFG_HOOK_PRE_THD_TERM) && (0U != CFG_HOOK_PRE_THD_TERM))
# error "NUB RT Kernel RT Kernel: Configuration option CFG_HOOK_PRE_THD_TERM is out of range."
#endif

#if ((1U != CFG_HOOK_PRE_THD_TERM) && (0U != CFG_HOOK_PRE_THD_TERM))
# error "NUB RT Kernel RT Kernel: Configuration option CFG_HOOK_PRE_THD_TERM is out of range."
#endif

#if ((1U != CFG_HOOK_PRE_IDLE) && (0U != CFG_HOOK_PRE_IDLE))
# error "NUB RT Kernel RT Kernel: Configuration option CFG_HOOK_PRE_IDLE is out of range."
#endif

#if ((1U != CFG_HOOK_PRE_CTX_SW) && (0U != CFG_HOOK_PRE_CTX_SW))
# error "NUB RT Kernel RT Kernel: Configuration option CFG_HOOK_PRE_CTX_SW is out of range."
#endif

#if (0 > CFG_SYSTMR_TICK_TYPE) || (2U < CFG_SYSTMR_TICK_TYPE)
# error "NUB RT Kernel RT Kernel: Configuration option CFG_SYSTMR_TICK_TYPE is out of range."
#endif

/** @endcond *//** @} *//******************************************************
 * END of kernel_config.h
 ******************************************************************************/
#endif /* KERNEL_CONFIG_H_ */
