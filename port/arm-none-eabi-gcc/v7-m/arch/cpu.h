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
 * @brief       Interface of ARM Cortex CPU module port.
 * @addtogroup  arm-none-eabi-gcc
 *********************************************************************//** @{ */
/**@defgroup    arm-none-eabi-gcc-v7-m-cpu ARM Cortex M3/M4 CPU module
 * @brief       CPU module
 * @{ *//*--------------------------------------------------------------------*/

#ifndef NCPU_H
#define NCPU_H

/*=========================================================  INCLUDE FILES  ==*/

#include <stdint.h>
#include <stdbool.h>

#include "plat/compiler.h"
#include "lib/natomic.h"

/*===============================================================  MACRO's  ==*/

/*------------------------------------------------------------------------*//**
 * @name        Port constants
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Specifies bit-width of general purpose registers
 */
#define NCPU_DATA_WIDTH                     32u

/**@brief       Specifies data alignment for optimal performance
 */
#define NCPU_DATA_ALIGNMENT                 4u


/**@} *//*--------------------------------------------------------------------------------------------------------*//**
 * @name        System timer constants
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

/**@brief       Threshold system timer value for new tick
 */
#define PORT_DEF_SYSTMR_WAKEUP_TH_VAL       600u

/**@} *//*-----------------------------------------------  C++ extern base  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/
/*======================================================  GLOBAL VARIABLES  ==*/
/*===================================================  FUNCTION PROTOTYPES  ==*/

/*------------------------------------------------------------------------*//**
 * @name        Bit operations
 * @{ *//*--------------------------------------------------------------------*/


/**@brief       Computes integer logarithm base 2
 * @inline
 */
static PORT_C_INLINE_ALWAYS uint_fast8_t ncpu_log2(
    n_native                    value)
{
    uint_fast8_t                clz;

    __asm__ __volatile__ (
        "@  ncpu_log2                                       \n"
        "   clz    %0, %1                                   \n"
        : "=r"(clz)
        : "r"(value));

    return (31u - clz);
}



/**@brief       Computes integer exponent base 2
 * @inline
 */
static PORT_C_INLINE_ALWAYS n_native ncpu_exp2(
    uint_fast8_t                value)
{
    return (0x1u << value);
}

/**@} *//*----------------------------------------------------------------*//**
 * @name        Atomic operations
 * @{ *//*--------------------------------------------------------------------*/


/**@brief       Atomically adds number to an atomic number
 * @inline
 */
static PORT_C_INLINE uint32_t ncpu_atomic_add(
    uint32_t                    number,
    struct natomic *            atomic)
{
    uint32_t                    result;
    n_native                    tmp;

    __asm__ __volatile__ (
        "@  ncpu_add                                        \n"
        "1: ldrex   %0, [%3]                                \n"
        "   add     %0, %0, %4                              \n"
        "   strex   %1, %0, [%3]                            \n"
        "   cmp     %1, #0                                  \n"
        "   bne     1b"
        : "=&r" (result), "=&r" (tmp), "+Qo" (atomic->counter)
        : "r" (&atomic->counter), "Ir" (number)
        : "cc");

    return (result);
}

/**@} *//*----------------------------------------------------------------*//**
 * @name        Generic port functions
 * @{ *//*--------------------------------------------------------------------*/


/**@brief       Initializes CPU module
 * @inline
 */
static PORT_C_INLINE void ncpu_module_init(
    void)
{
    __asm__ __volatile__(
        "@  ncpu_init                                       \n"
        "   clrex                                           \n");               /* Clear the exclusive monitor.       */
}

#define ncpu_module_term()                  (void)0

/** @} *//*-----------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of cpu.h
 ******************************************************************************/
#endif /* NCPU_H */
