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
 * @brief   	Configuration of CPU module.
 * @addtogroup  arm-none-eabi-gcc-v7-m_cfg
 * @brief		Configuration of CPU module.
 *********************************************************************//** @{ */

#if !defined(CPU_CONFIG_H_)
#define CPU_CONFIG_H_

/*=========================================================  INCLUDE FILES  ==*/
/*===============================================================  DEFINES  ==*/
/** @cond */

#define CPU_STM32F10X
#define CFG_SYSTMR_CLOCK_FREQUENCY      8000000UL

/** @endcond */
/*==============================================================  SETTINGS  ==*/

/*------------------------------------------------------------------------*//**
 * @name        ST Microelectronics STM32F10x
 * @{ *//*--------------------------------------------------------------------*/
#if defined(CPU_STM32F10X) || defined(__DOXYGEN__)

#define CPU_FOUND_                                                              /**< @brief Note that a port is found.                      */

/**@brief       Priority of critical sections in kernel
 * @details     Specify the priority in range: <code>0</code>\f$\leq\f$
 *              <code>CFG_CRITICAL_PRIO</code> \f$\leq\f$ <code>15</code>. The
 *              lower the number the higher the priority.
 * @note        When priority is set to @b 0 then critical section will not use
 *              priority levels bit it will just disable interrupts on entry and
 *              enable interrupts on exit.
 */
#if !defined(CFG_CRITICAL_PRIO) || defined(__DOXYGEN__)
# define CFG_CRITICAL_PRIO              7U
#endif

/**@brief       The frequency of clock which is used for the system timer
 * @details     System timer SysTick uses core clock (sometimes referred to as
 *              HCLK) for counting. Specify here the core clock so the OS can
 *              properly manage system tick event generation.
 */
#if !defined(CFG_SYSTMR_CLOCK_FREQUENCY) || defined(__DOXYGEN__)
# define CFG_SYSTMR_CLOCK_FREQUENCY     24000000UL
#endif

/**@brief       Port constant: interrupt priority bits implemented in MCU
 * @note        It is also recommended to ensure that all priority bits are
 *              assigned as being preemption priority bits, and none as sub
 *              priority bits
 */
# define CPU_ISR_PRIO_BITS              4U

#endif

/** @} *//*---------------------------------------------------------------*//**
 * @name        Exception handlers used by the port
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       PendSV exception handler
 */
#if !defined(portPendSV) || defined(__DOXYGEN__)
# define portPendSV                     PendSV_Handler
#endif

/**@brief       SVC exception handler
 */
#if !defined(portSVC) || defined(__DOXYGEN__)
# define portSVC                        SVC_Handler
#endif

/**@brief       SysTick exception handler
 */
#if !defined(portSysTmr) || defined(__DOXYGEN__)
# define portSysTmr                     SysTick_Handler
#endif

/** @} *//*-------------------------------------------------------------------*/
/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/

#if !defined(CPU_FOUND_)
# error "eSolid RT Kernel port: please define a valid port macro."
#endif

/** @endcond *//** @} *//******************************************************
 * END of cpu_cfg.h
 ******************************************************************************/
#endif /* CPU_CONFIG_H_ */
