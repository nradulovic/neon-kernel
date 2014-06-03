/*
 * This file is part of nKernel.
 *
 * Copyright (C) 2010 - 2013 Nenad Radulovic
 *
 * nKernel is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * nKernel is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with nKernel.  If not, see <http://www.gnu.org/licenses/>.
 *
 * web site:    http://github.com/nradulovic
 * e-mail  :    nenad.b.radulovic@gmail.com
 *//***********************************************************************//**
 * @file
 * @author  	Nenad Radulovic
 * @brief   	Family profile for ST-Microelectronics STM32F10x
 * @addtogroup  arm-none-eabi-gcc
 * @brief       Family profile for ST-Microelectronics STM32F10x
 *********************************************************************//** @{ */
/**@defgroup    arm-none-eabi-gcc-stm32f10x ST-Microelectronics STM32F10x
 * @brief       ST-Microelectronics STM32F10x
 * @{ *//*--------------------------------------------------------------------*/

#ifndef ES_PROFILE_H_
#define ES_PROFILE_H_

/*=========================================================  INCLUDE FILES  ==*/

#include "arch/cortex_m3.h"

/*===============================================================  MACRO's  ==*/

/**@brief       Specifies maximum CPU clock speed in Hz.
 */
#define ES_PROFILE_MAX_CPU_CLOCK        24000000ul

/**@brief       System timer maximum value
 * @details     STM32F10x family has 24-bit wide system tick register
 */
#define ES_PROFILE_MAX_SYSTIMER_VAL     0xfffffful

/**@brief       Maximum RAM size for this family
 * @details     This define is used to choose optimal algorithm for this family
 *              of micro-controllers.
 */
#define ES_PROFILE_MAX_RAM_SIZE         8192u

#if !defined(ES_OPTIMIZE_FOR_SPEED)
#define ES_RAM_SIZE_MAX                 65535
#define ES_RAM_SSIZE_MAX                32767
#define ES_RAM_SSIZE_MIN                -32768
#else
#define ES_RAM_SIZE_MAX                 4294967295ul
#define ES_RAM_SSIZE_MAX                2147483647l
#define ES_RAM_SSIZE_MIN                -2147483648l
#endif /* !ES_OPTIMIZE_FOR_SPEED */

/**@brief       Port constant: interrupt priority bits implemented in MCU
 * @note        It is also recommended to ensure that all priority bits are
 *              assigned as being preemption priority bits, and none as sub
 *              priority bits
 */
#define PORT_ISR_PRIO_BITS              4u

/*-------------------------------------------------------  C++ extern base  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/

typedef unsigned int       esRamSize;
typedef signed   int       esRamSSize;

/*======================================================  GLOBAL VARIABLES  ==*/
/*===================================================  FUNCTION PROTOTYPES  ==*/
/*--------------------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif
/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//** @} *//*********************************************
 * END of profile.h
 ******************************************************************************/
#endif /* ES_PROFILE_H_ */
