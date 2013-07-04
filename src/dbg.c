/*
 * This file is part of eSolid-Kernel
 *
 * Copyright (C) 2011, 2012 - Nenad Radulovic
 *
 * eSolid-Kernel is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * eSolid-Kernel is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with eSolid-Kernel; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 *
 * web site:    http://blueskynet.dyndns-server.com
 * e-mail  :    blueskyniss@gmail.com
 *//***********************************************************************//**
 * @file
 * @author      Nenad Radulovic
 * @brief       Implementation of Debug module
 * @addtogroup  dbg_impl
 *********************************************************************//** @{ */

/*=========================================================  INCLUDE FILES  ==*/
#include "kernel.h"

/*=========================================================  LOCAL MACRO's  ==*/
/*======================================================  LOCAL DATA TYPES  ==*/
/*=============================================  LOCAL FUNCTION PROTOTYPES  ==*/
/*=======================================================  LOCAL VARIABLES  ==*/
/*======================================================  GLOBAL VARIABLES  ==*/
/*============================================  LOCAL FUNCTION DEFINITIONS  ==*/
/*===================================  GLOBAL PRIVATE FUNCTION DEFINITIONS  ==*/
/*====================================  GLOBAL PUBLIC FUNCTION DEFINITIONS  ==*/

/* 1)       This function will disable all interrupts to prevent any new
 *          interrupt to execute which can trigger another assert causing a very
 *          confusing situation of why it failed.
 */
PORT_C_NORETURN void esDbgAssert(
    const char *    fnName,
    const char *    expr,
    enum esDbgMsg  msg) {

    const char * assertText;

    PORT_INT_DISABLE();
    switch (msg) {

        case ES_DBG_OUT_OF_RANGE : {
            assertText = "Value is out of valid range";
            break;
        }

        case ES_DBG_OBJECT_NOT_VALID : {
            assertText = "Object is not valid";
            break;
        }

        case ES_DBG_POINTER_NULL : {
            assertText = "Pointer has NULL value";
            break;
        }

        case ES_DBG_USAGE_FAILURE : {
            assertText = "Object usage failure";
            break;
        }

        case ES_DBG_NOT_ENOUGH_MEM : {
            assertText = "Not enough memory available";
            break;
        }

        default : {
            assertText = "Unknown error has occured";
            break;
        }

    }
    userAssert(
        fnName,
        assertText,
        expr);

    while (TRUE);
}

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of dbg.c
 ******************************************************************************/
