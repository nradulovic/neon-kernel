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
 * @brief       Interface of ARM Cortex CPU module port.
 * @addtogroup  arm-none-eabi-gcc
 *********************************************************************//** @{ */
/**@defgroup    arm-none-eabi-gcc-v7-m-cpu ARM Cortex M3/M4 CPU module
 * @brief       CPU module
 * @{ *//*--------------------------------------------------------------------*/

#ifndef ES_CPU_H_
#define ES_CPU_H_

/*=========================================================  INCLUDE FILES  ==*/

#include <stdint.h>
#include "plat/compiler.h"

/*===============================================================  MACRO's  ==*/

/*------------------------------------------------------------------------*//**
 * @name        Port constants
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Specifies bit-width of general purpose registers
 */
#define ES_CPU_DEF_DATA_WIDTH           32u

/**@brief       Specifies data alignment for optimal performance
 */
#define ES_CPU_DEF_DATA_ALIGNMENT       4u

/**@} *//*----------------------------------------------------------------*//**
 * @name        Bit operations
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Find Last Set bit in a word
 */
#define ES_CPU_FLS(val)                 portCpuFls_(val)

/**@brief       Compute power of 2
 */
#define ES_CPU_PWR2(pwr)                (0x01u << (pwr))

/**@} *//*----------------------------------------------------------------*//**
 * @name        Generic port macros
 * @{ *//*--------------------------------------------------------------------*/

#define ES_MODULE_CPU_INIT()            portModuleCpuInit()

#define ES_MODULE_CPU_TERM()            portModuleCpuTerm()

#define ES_CPU_INIT()                   (void)0

#define ES_CPU_INIT_LATE()              (void)0

#define ES_CPU_TERM()                   (void)0

/**@} *//*----------------------------------------------------------------*//**
 * @name        CPU register descriptions
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       PSR Thumb state: Position.
 */
#define PORT_PSR_THUMB_STATE_Pos        (24u)

/**@brief       PSR Thumb state: Mask.
 */
#define PORT_PSR_THUMB_STATE_Msk        (0x01ul << PORT_PSR_THUMB_STATE_Pos)

/**@} *//*-----------------------------------------------  C++ extern base  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/

/**@brief       General purpose registers are 32bit wide.
 */
typedef unsigned int esCpuReg;

/*======================================================  GLOBAL VARIABLES  ==*/
/*===================================================  FUNCTION PROTOTYPES  ==*/

/*------------------------------------------------------------------------*//**
 * @name        Bit operations
 * @{ *//*--------------------------------------------------------------------*/

/**
 * @brief       Find last set bit in a word
 * @param       value
 *              32 bit value which will be evaluated
 * @return      Last set bit in a word
 * @details     This implementation uses @c clz instruction and then it computes
 *              the result using the following expression:
 *              <code>fls(x) = w − clz(x)</code>.
 * @inline
 */
static PORT_C_INLINE_ALWAYS uint_fast8_t portCpuFls_(
    natomic            value) {

    uint_fast8_t        clz;

    __asm__ __volatile__ (
        "   clz    %0, %1                                   \n"
        : "=r"(clz)
        : "r"(value));

    return (31u - clz);
}

/** @} *//*---------------------------------------------------------------*//**
 * @name        Generic port functions
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Initialize CPU port module
 */
void portModuleCpuInit(
    void);

/**@brief       Terminate CPU port module
 */
void portModuleCpuTerm(
    void);

/** @} *//*-----------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of cpu.h
 ******************************************************************************/
#endif /* ES_CPU_H_ */
