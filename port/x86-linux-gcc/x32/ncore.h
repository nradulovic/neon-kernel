/*
 * This file is part of Neon RT Kernel.
 *
 * Copyright (C) 2010 - 2014 Nenad Radulovic
 *
 * Neon RT Kernel is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Neon RT Kernel is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Neon RT Kernel.  If not, see <http://www.gnu.org/licenses/>.
 *
 * web site:    http://github.com/nradulovic
 * e-mail  :    nenad.b.radulovic@gmail.com
 *//***********************************************************************//**
 * @file
 * @author  	Nenad Radulovic
 * @brief       Port core module header
 * @addtogroup  x86-linux-gcc
 *********************************************************************//** @{ */
/**@defgroup    x86-linux-gcc-core x86 Linux GCC Core module
 * @brief       Port core module
 * @{ *//*--------------------------------------------------------------------*/

#ifndef NPORT_CORE_H
#define NPORT_CORE_H

/*=========================================================  INCLUDE FILES  ==*/

#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>

#include "plat/compiler.h"
#include "plat/sys_lock.h"
#include "family/profile.h"
#include "arch/ncore_config.h"

/*===============================================================  MACRO's  ==*/

/*------------------------------------------------------------------------*//**
 * @name        CPU management macros
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Specifies bit-width of general purpose registers
 */
#define NCPU_DATA_WIDTH                     32u

/**@brief       Specifies data alignment for optimal performance
 */
#define NCPU_DATA_ALIGNMENT                 4u

#define NCPU_REG_MAX                        UINT32_MAX

/**@} *//*----------------------------------------------------------------*//**
 * @name        Interrupt service management macros
 * @{ *//*--------------------------------------------------------------------*/

#define NISR_PRIO_TO_CODE(prio)             prio

#define NISR_CODE_TO_PRIO(code)             code
    
#define nisr_pend_kernel_isr_i              (void)0

/**@} *//*----------------------------------------------------------------*//**
 * @name        Generic port functions
 * @{ *//*--------------------------------------------------------------------*/
 
#define ncore_init()                        (void)0

#define ncore_term()                        (void)0

/**@} *//*-----------------------------------------------  C++ extern base  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/

/**@brief General purpose registers are 32bit wide.
 */
typedef unsigned int ncpu_reg;

/**@brief       Core timer hardware register type.
 */
typedef unsigned int ncore_timer_tick;

/*======================================================  GLOBAL VARIABLES  ==*/
/*===================================================  FUNCTION PROTOTYPES  ==*/

/*------------------------------------------------------------------------*//**
 * @name        CPU arithmetic/logic operations
 * @{ *//*--------------------------------------------------------------------*/


/**@brief       Stop the further CPU execution
 */
PORT_C_INLINE
void ncpu_stop(void)
{
    while (true);
}



/**@brief       Computes integer logarithm base 2
 */
PORT_C_INLINE_ALWAYS
uint_fast8_t ncpu_log2(
    ncpu_reg                    value)
{
    uint_fast8_t                clz;

    clz = 0;
    
    return (31u - clz);
}



/**@brief       Computes integer exponent base 2
 */
PORT_C_INLINE_ALWAYS
ncpu_reg ncpu_exp2(
    uint_fast8_t                value)
{
    return (0x1u << value);
}



PORT_C_INLINE_ALWAYS
void ncpu_sat_increment(
    ncpu_reg *                  value)
{
    if (*value != NCPU_REG_MAX) {
        (*value)++;
    }
}



PORT_C_INLINE_ALWAYS
void ncpu_sat_decrement(
    ncpu_reg *                  value)
{
    if (*value != 0u) {
        (*value)--;
    }
}

/**@} *//*----------------------------------------------------------------*//**
 * @name        Generic port functions
 * @{ *//*--------------------------------------------------------------------*/


extern void ncore_timer_isr(void);

/** @} *//*-----------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//** @} *//*********************************************
 * END of nport_core.h
 ******************************************************************************/
#endif /* NPORT_CORE_H */
