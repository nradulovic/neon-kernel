/*
 * This file is part of eSolid-Kernel
 *
 * Copyright (C) 2011, 2012, 2013 - Nenad Radulovic
 *
 * eSolid-Kernel is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option) any
 * later version.
 *
 * eSolid-Kernel is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * eSolid-Kernel; if not, write to the Free Software Foundation, Inc., 51
 * Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * web site:    http://github.com/nradulovic
 * e-mail  :    nenad.b.radulovic@gmail.com
 *//***********************************************************************//**
 * @file
 * @author  	Nenad Radulovic
 * @brief   	Family profile for STM32F10x
 * @addtogroup  port_family_stm32f10x
 * @brief       Family profile for STM32F10x
 *********************************************************************//** @{ */

#if !defined(PROFILE_H__)
#define PROFILE_H__

/*=========================================================  INCLUDE FILES  ==*/
/*===============================================================  DEFINES  ==*/
/*==============================================================  SETTINGS  ==*/

/*------------------------------------------------------------------------*//**
 * @name        ST-Microelectronics STM32F10x
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Port constant: interrupt priority bits implemented in MCU
 * @note        It is also recommended to ensure that all priority bits are
 *              assigned as being preemption priority bits, and none as sub
 *              priority bits
 */
#define CPU_DEF_ISR_PRIO_BITS           4u

/**@brief       System timer maximum value
 * @details     STM32F10x family has 24-bit wide system tick register
 */
#define CPU_DEF_SYSTMR_MAX_VAL          0xfffffful

/**@} *//*----------------------------------------------------------------*//**
 * @name        Exception handlers used by the port
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       PendSV exception handler
 */
#if !defined(portPendSV)
# define portPendSV                     PendSV_Handler
#endif

/**@brief       SVC exception handler
 */
#if !defined(portSVC)
# define portSVC                        SVC_Handler
#endif

/**@brief       SysTick exception handler
 */
#if !defined(portSysTmr)
# define portSysTmr                     SysTick_Handler
#endif

/** @} *//*-------------------------------------------------------------------*/
/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of profile.h
 ******************************************************************************/
#endif /* PROFILE_H__ */
