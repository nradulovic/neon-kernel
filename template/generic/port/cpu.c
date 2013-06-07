/******************************************************************************
 * This file is part of esolid-rtos
 *
 * Copyright (C) 2011, 2012 - Nenad Radulovic
 *
 * esolid-rtos is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * esolid-rtos is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with esolid-rtos; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 *
 * web site:    http://blueskynet.dyndns-server.com
 * e-mail  :    blueskyniss@gmail.com
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

portReg_T gPortIsrNesting_;

const PORT_C_ROM portReg_T pwr2LKP [PORT_DATA_WIDTH_VAL] = {
    (1U <<  0), (1U <<  1), (1U <<  2), (1U <<  3),
    (1U <<  4), (1U <<  5), (1U <<  6), (1U <<  7),
#if (8U < PORT_DATA_WIDTH)
    (1U <<  8), (1U <<  9), (1U << 10), (1U << 11),
    (1U << 12), (1U << 13), (1U << 14), (1U << 15),
# if (16U < PORT_DATA_WIDTH)
    (1U << 16), (1U << 17), (1U << 18), (1U << 19),
    (1U << 20), (1U << 21), (1U << 22), (1U << 23),
    (1U << 24), (1U << 25), (1U << 26), (1U << 27),
    (1U << 28), (1U << 29), (1U << 30), (1U << 31)
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
uint_fast8_t portFindLastSet_(
    portReg_T       val) {

    portReg_T       tmp;
    uint_fast8_t    rtn;

    rtn = 0U;

#if (32U == PORT_DATA_WIDTH_VAL)
    tmp = val >> 16;

    if (0U != tmp) {
        val = tmp;
        rtn = 16U;
    }
#endif

#if (16U <= PORT_DATA_WIDTH_VAL)
    tmp = val >> 8;

    if (0U != tmp) {
        val = tmp;
        rtn += 8U;
    }
#endif
    tmp = val >> 4;

    if (0U != tmp) {
        val = tmp;
        rtn += 4U;
    }
    tmp = val >> 2;

    if (0U != tmp) {
        val = tmp;
        rtn += 2U;
    }
    tmp = val >> 1;

    if (0U != tmp) {
        val = tmp;
        rtn += 1U;
    }

    return (rtn);
}

void * portCtxInit_(
    void *          stck,
    size_t          stckSize,
    void (* thdf)(void *),
    void *          arg) {

    (void)stck;
    (void)stckSize;
    (void)thdf;
    (void)arg;

    return (NULL);
}

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of cpu.c
 ******************************************************************************/
