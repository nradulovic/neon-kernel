/*
 * This file is part of eSolid-Kernel
 *
 * Copyright (C) 2013 - Nenad Radulovic
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
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Scheduler priority levels
 * @details     Possible values:
 *              - Min: 3U (three priority levels)
 *              - Max: 256U
 */
#if !defined(CFG_SCHED_PRIO_LVL)
# define CFG_SCHED_PRIO_LVL             8U
#endif

/**@brief       Scheduler Round-Robin time quantum
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

/**@brief       System timer adaptive mode
 * @details     Possible values are:
 *              - 0U - adaptive mode is disabled
 *              - 1U - adaptive mode is enabled
 */
#if !defined(CFG_SYSTMR_ADAPTIVE_MODE)
# define CFG_SYSTMR_ADAPTIVE_MODE       0U
#endif

/**@brief       The frequency of system timer tick event
 * @note        This setting is valid only if configuration option
 *              @ref CFG_SYSTMR_CLOCK_FREQUENCY is properly set in port
 *              configuration file cpu_cfg.h
 */
#if !defined(CFG_SYSTMR_EVENT_FREQUENCY)
# define CFG_SYSTMR_EVENT_FREQUENCY     100UL
#endif

/**@brief       The size of the system timer tick event counter
 * @details     Possible values are:
 *              - 0U - 8 bit counter
 *              - 1U - 16 bit counter
 *              - 2U - 32 bit counter
 */
#if !defined(CFG_SYSTMR_TICK_TYPE)
# define CFG_SYSTMR_TICK_TYPE           2U
#endif

/** @} *//*---------------------------------------------------------------*//**
 * @name        Kernel pre hooks
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       System timer event hook function
 */
#if !defined(CFG_HOOK_PRE_SYSTMR_EVENT)
# define CFG_HOOK_PRE_SYSTMR_EVENT      0U
#endif

/**@brief       Pre kernel initialization hook function
 */
#if !defined(CFG_HOOK_PRE_KERN_INIT)
# define CFG_HOOK_PRE_KERN_INIT         0U
#endif

/**@brief       Post kernel initialization hook function
 */
#if !defined(CFG_HOOK_PORT_KERN_INIT)
# define CFG_HOOK_POST_KERN_INIT        0U
#endif

/**@brief       Pre kernel start hook function
 */
#if !defined(CFG_HOOK_PRE_KERN_START)
# define CFG_HOOK_PRE_KERN_START        0U
#endif

/**@brief       Post thread initialization hook function
 */
#if !defined(CFG_HOOK_POST_THD_INIT)
# define CFG_HOOK_POST_THD_INIT         0U
#endif

/**@brief       Pre thread termination hook function
 */
#if !defined(CFG_HOOK_PRE_THD_TERM)
# define CFG_HOOK_PRE_THD_TERM          0U
#endif

/**@brief       Pre idle hook function
 */
#if !defined(CFG_HOOK_PRE_IDLE)
# define CFG_HOOK_PRE_IDLE              0U
#endif

/**@brief       Pre context switch hook function
 */
#if !defined(CFG_HOOK_PRE_CTX_SW)
# define CFG_HOOK_PRE_CTX_SW            0U
#endif

/** @} *//*-------------------------------------------------------------------*/
/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/

#if ((3U > CFG_SCHED_PRIO_LVL) || (256U < CFG_SCHED_PRIO_LVL))
# error "eSolid RT Kernel: Configuration option CFG_SCHED_PRIO_LVL is out of range."
#endif

#if ((1U != CFG_SCHED_POWER_SAVE) && (0U != CFG_SCHED_POWER_SAVE))
# error "eSolid RT Kernel: Configuration option CFG_SCHED_POWER_SAVE is out of range."
#endif

#if ((1U != CFG_SYSTMR_ADAPTIVE_MODE) && (0U != CFG_SYSTMR_ADAPTIVE_MODE))
# error "eSolid RT Kernel: Configuration option CFG_SYSTMR_ADAPTIVE_MODE is out of range."
#endif

#if ((0U == CFG_SCHED_POWER_SAVE) && (1U == CFG_SYSTMR_ADAPTIVE_MODE))
# error "eSolid RT Kernel: Configuration option CFG_SCHED_PRIO_LVL must be enabled when CFG_SYSTMR_ADAPTIVE_MODE is enabled, too."
#endif

#if ((1U != CFG_HOOK_PRE_SYSTMR_EVENT) && (0U != CFG_HOOK_PRE_SYSTMR_EVENT))
# error "eSolid RT Kernel: Configuration option CFG_HOOK_PRE_SYSTMR_EVENT is out of range."
#endif

#if ((1U != CFG_HOOK_PRE_KERN_INIT) && (0U != CFG_HOOK_PRE_KERN_INIT))
# error "eSolid RT Kernel: Configuration option CFG_HOOK_PRE_KERN_INIT is out of range."
#endif

#if ((1U != CFG_HOOK_POST_KERN_INIT) && (0U != CFG_HOOK_POST_KERN_INIT))
# error "eSolid RT Kernel: Configuration option CFG_HOOK_POST_KERN_INIT is out of range."
#endif

#if ((1U != CFG_HOOK_PRE_KERN_START) && (0U != CFG_HOOK_PRE_KERN_START))
# error "eSolid RT Kernel: Configuration option CFG_HOOK_PRE_KERN_START is out of range."
#endif

#if ((1U != CFG_HOOK_POST_THD_INIT) && (0U != CFG_HOOK_POST_THD_INIT))
# error "eSolid RT Kernel: Configuration option CFG_HOOK_POST_THD_INIT is out of range."
#endif

#if ((1U != CFG_HOOK_PRE_THD_TERM) && (0U != CFG_HOOK_PRE_THD_TERM))
# error "eSolid RT Kernel: Configuration option CFG_HOOK_PRE_THD_TERM is out of range."
#endif

#if ((1U != CFG_HOOK_PRE_THD_TERM) && (0U != CFG_HOOK_PRE_THD_TERM))
# error "eSolid RT Kernel: Configuration option CFG_HOOK_PRE_THD_TERM is out of range."
#endif

#if ((1U != CFG_HOOK_PRE_IDLE) && (0U != CFG_HOOK_PRE_IDLE))
# error "eSolid RT Kernel: Configuration option CFG_HOOK_PRE_IDLE is out of range."
#endif

#if ((1U != CFG_HOOK_PRE_CTX_SW) && (0U != CFG_HOOK_PRE_CTX_SW))
# error "eSolid RT Kernel: Configuration option CFG_HOOK_PRE_CTX_SW is out of range."
#endif

#if (0 > CFG_SYSTMR_TICK_TYPE) || (2U < CFG_SYSTMR_TICK_TYPE)
# error "eSolid RT Kernel: Configuration option CFG_SYSTMR_TICK_TYPE is out of range."
#endif

/** @endcond *//** @} *//******************************************************
 * END of kernel_config.h
 ******************************************************************************/
#endif /* KERNEL_CONFIG_H_ */
