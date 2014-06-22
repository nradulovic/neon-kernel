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
 * @brief   	Configuration of CPU module.
 * @addtogroup  arm-none-eabi-gcc-v7-m_cfg
 * @brief		Configuration of CPU module.
 *************************************************************************************************************//** @{ */

#ifndef NCPU_CONFIG_H_
#define NCPU_CONFIG_H_

/*=================================================================================================  INCLUDE FILES  ==*/
/*=======================================================================================================  DEFINES  ==*/
/*======================================================================================================  SETTINGS  ==*/

/**@brief       This is the data that will be placed on task context at its creation
 * @details     This macro can be used if you need to specify different settings for Interruptible-continuable
 *              instructions. The setting is done in PSR register.
 */
#define PORT_CONFIG_CPU_PSR_DATA            0u

/*========================================================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//**********************************************************************************************
 * END of cpu_config.h
 **********************************************************************************************************************/
#endif /* NCPU_CONFIG_H_ */
