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
 *//***************************************************************************************************************//**
 * @file
 * @author      Nenad Radulovic
 * @brief       Implementation of ARM Cortex-M3 System Timer port.
 * @addtogroup  arm-none-eabi-systimer
 *********************************************************************//** @{ */
/**@defgroup    arm-none-eabi-systimer_impl System Timer module Implementation
 * @brief       System Timer module Implementation
 * @{ *//*--------------------------------------------------------------------*/

/*=========================================================  INCLUDE FILES  ==*/

#include <stddef.h>

#include "arch/systimer.h"
#include "arch/intr.h"
#include "lib/nbitop.h"
#include "lib/ndebug.h"

/*=========================================================  LOCAL MACRO's  ==*/
/*======================================================  LOCAL DATA TYPES  ==*/
/*=============================================  LOCAL FUNCTION PROTOTYPES  ==*/
/*=======================================================  LOCAL VARIABLES  ==*/

static const NMODULE_INFO_CREATE("System Timer (port)", "Nenad Radulovic");

static void (* g_timer_handler[4])(void);

/*======================================================  GLOBAL VARIABLES  ==*/
/*============================================  LOCAL FUNCTION DEFINITIONS  ==*/
/*===================================  GLOBAL PRIVATE FUNCTION DEFINITIONS  ==*/
/*====================================  GLOBAL PUBLIC FUNCTION DEFINITIONS  ==*/


void ntimer_module_init(
    void)
{
    ntimer_disable();
    /*
     * TODO: Clear interrupt flag and interrupt enable bits
     * TODO: Set up ISR priority
     */
}



void ntimer_module_term(
    void)
{
    ntimer_disable();
    /*
     * TODO: Clear interrupt flag and interrupt enable bits
     */
}

void ntimer_set_handler(
    void                     (* handler)(void),
    uint_fast8_t                level)
{
    NREQUIRE(NAPI_RANGE, level < NARRAY_DIMENSION(g_timer_handler));

    g_timer_handler[level] = handler;
}



void portSysTimerHandler(
    void)
{
    uint_fast8_t        count;

    for (count = 0; count < NARRAY_DIMENSION(g_timer_handler); count++) {
        if (g_timer_handler[count] != NULL) {
            g_timer_handler[count]();
        }
    }
    /*
     * TODO: Clear interrupt flag
     */
}

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//** @} *//*********************************************
 * END of cpu.c
 ******************************************************************************/

