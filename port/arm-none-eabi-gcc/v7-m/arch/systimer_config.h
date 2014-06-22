/*
 * This file is part of NUB RT Kernel.
 *
 * Copyright (C) 2010 - 2013 Nenad Radulovic
 *
 * NUB RT Kernel is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * NUB RT Kernel is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with NUB RT Kernel.  If not, see <http://www.gnu.org/licenses/>.
 *
 * web site:    http://github.com/nradulovic
 * e-mail  :    nenad.b.radulovic@gmail.com
 *//***********************************************************************//**
 * @file
 * @author  	Nenad Radulovic
 * @brief   	Configuration of system timer module.
 * @addtogroup  arm-none-eabi-gcc-v7-m_cfg
 * @brief		Configuration of system timer module.
 *********************************************************************//** @{ */

#ifndef ES_ARCH_SYSTIMER_CFG_H_
#define ES_ARCH_SYSTIMER_CFG_H_

/*=========================================================  INCLUDE FILES  ==*/

#include "family/profile.h"

/*===============================================================  DEFINES  ==*/
/*==============================================================  SETTINGS  ==*/

/**@brief       The frequency of clock which is used for the system timer
 * @details     System timer SysTick uses core clock (sometimes referred to as
 *              HCLK) for counting. Specify here the core clock so the OS can
 *              properly manage system tick event generation.
 */
#if !defined(CONFIG_SYSTIMER_CLOCK_FREQ)
# define CONFIG_SYSTIMER_CLOCK_FREQ     24000000ul
#endif

/**@brief       The frequency of system timer tick event
 * @note        This setting is valid only if configuration option
 *              @ref CONFIG_SYSTIMER_CLOCK_FREQ is properly set.
 */
#if !defined(CONFIG_SYSTIMER_EVENT_FREQ)
# define CONFIG_SYSTIMER_EVENT_FREQ     100ul
#endif

#if !defined(portSysTimerHandler)
# define portSysTimerHandler		    SysTick_Handler
#endif

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of systimer_config.h
 ******************************************************************************/
#endif /* ES_ARCH_SYSTIMER_CFG_H_ */
