/*
 * This file is part of eSolid - RT Kernel
 *
 * Copyright (C) 2011, 2012, 2013 - Nenad Radulovic
 *
 * eSolid - RT Kernel is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option) any
 * later version.
 *
 * eSolid - RT Kernel is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * eSolid - RT Kernel; if not, write to the Free Software Foundation, Inc., 51
 * Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * web site:    http://github.com/nradulovic
 * e-mail  :    nenad.b.radulovic@gmail.com
 *//***********************************************************************//**
 * @file
 * @author      Nenad Radulovic
 * @brief       Implementation of CPU port - Template
 * @addtogroup  template_cpu_impl
 *********************************************************************//** @{ */

/*=========================================================  INCLUDE FILES  ==*/

#include "kernel.h"

/*=========================================================  LOCAL MACRO's  ==*/
/*======================================================  LOCAL DATA TYPES  ==*/
/*=============================================  LOCAL FUNCTION PROTOTYPES  ==*/
/*=======================================================  LOCAL VARIABLES  ==*/
/*======================================================  GLOBAL VARIABLES  ==*/

portReg_T PortIsrNesting;

const PORT_C_ROM portReg_T Pwr2LKP [PORT_DEF_DATA_WIDTH] = {
    (1u <<  0), (1u <<  1), (1u <<  2), (1u <<  3),
    (1u <<  4), (1u <<  5), (1u <<  6), (1u <<  7),
#if (8u < PORT_DEF_DATA_WIDTH)
    (1u <<  8), (1u <<  9), (1u << 10), (1u << 11),
    (1u << 12), (1u << 13), (1u << 14), (1u << 15),
# if (16u < PORT_DEF_DATA_WIDTH)
    (1u << 16), (1u << 17), (1u << 18), (1u << 19),
    (1u << 20), (1u << 21), (1u << 22), (1u << 23),
    (1u << 24), (1u << 25), (1u << 26), (1u << 27),
    (1u << 28), (1u << 29), (1u << 30), (1u << 31)
# endif
#endif
};

/*============================================  LOCAL FUNCTION DEFINITIONS  ==*/
/*===================================  GLOBAL PRIVATE FUNCTION DEFINITIONS  ==*/
/*====================================  GLOBAL PUBLIC FUNCTION DEFINITIONS  ==*/

/*
 * This is a generic implementation of FLS algorthm and it should work on every
 * port available.
 */
uint_fast8_t portFindLastSet(
    portReg_T           val) {

    portReg_T           tmp;
    uint_fast8_t        ret;

    ret = 0u;

#if (32u == PORT_DEF_DATA_WIDTH)
    tmp = val >> 16;

    if (0u != tmp) {
        val = tmp;
        ret = 16u;
    }
#endif

#if (16u <= PORT_DEF_DATA_WIDTH)
    tmp = val >> 8;

    if (0u != tmp) {
        val = tmp;
        ret += 8u;
    }
#endif
    tmp = val >> 4;

    if (0u != tmp) {
        val = tmp;
        ret += 4u;
    }
    tmp = val >> 2;

    if (0u != tmp) {
        val = tmp;
        ret += 2u;
    }
    tmp = val >> 1;

    if (0u != tmp) {
        val = tmp;
        ret += 1u;
    }

    return (ret);
}

void * portCtxInit(
    void *              stck,
    size_t              stckSize,
    void (* fn)(void *),
    void *              arg) {

    (void)stck;
    (void)stckSize;
    (void)fn;
    (void)arg;

    return (NULL);
}

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of cpu.c
 ******************************************************************************/
