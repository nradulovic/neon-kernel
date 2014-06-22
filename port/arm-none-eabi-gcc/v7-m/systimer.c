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
#include "kernel/nbitop.h"
#include "kernel/ndebug.h"

/*=========================================================  LOCAL MACRO's  ==*/
/*======================================================  LOCAL DATA TYPES  ==*/
/*=============================================  LOCAL FUNCTION PROTOTYPES  ==*/
/*=======================================================  LOCAL VARIABLES  ==*/

static const NMODULE_INFO_CREATE("systimer", "System Timer (port)", "Nenad Radulovic");

static void (* GlobalSysTimerHandler[4])(void);

/*======================================================  GLOBAL VARIABLES  ==*/
/*============================================  LOCAL FUNCTION DEFINITIONS  ==*/
/*===================================  GLOBAL PRIVATE FUNCTION DEFINITIONS  ==*/
/*====================================  GLOBAL PUBLIC FUNCTION DEFINITIONS  ==*/

void portModuleSysTimerInit(
    void) {

    ES_SYSTIMER_DISABLE();
    /*
     * TODO: Clear interrupt flag and interrupt enable bits
     * TODO: Set up ISR priority
     */
}

void portModuleSysTimerTerm(
    void) {

    ES_SYSTIMER_DISABLE();
    /*
     * TODO: Clear interrupt flag and interrupt enable bits
     */
}

void portSysTimerSetHandler(
    void             (* handler)(void),
    uint_fast8_t        level) {

    NREQUIRE(NAPI_RANGE, level < NARRAY_DIMENSION(GlobalSysTimerHandler));

    GlobalSysTimerHandler[level] = handler;
}



void portSysTimerHandler(
    void) {

    uint_fast8_t        count;

    for (count = 0; count < NARRAY_DIMENSION(GlobalSysTimerHandler); count++) {
        if (GlobalSysTimerHandler[count] != NULL) {
            GlobalSysTimerHandler[count]();
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

