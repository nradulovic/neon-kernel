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

#if !defined(CPU_CFG_H_)
#define CPU_CFG_H_

/*=========================================================  INCLUDE FILES  ==*/
/*===============================================================  DEFINES  ==*/
/*==============================================================  SETTINGS  ==*/

/*------------------------------------------------------------------------*//**
 * @name        Exception handlers used by the local port
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
 * END of cpu_cfg.h
 ******************************************************************************/
#endif /* CPU_CFG_H_ */
