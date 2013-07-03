/*
 * This file is part of esolid-kernel
 *
 * Template version: 1.1.15 (03.07.2013)
 *
 * Copyright (C) 2011, 2012 - Nenad Radulovic
 *
 * esolid-kernel is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * esolid-kernel is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with esolid-kernel; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 *
 * web site:    http://blueskynet.dyndns-server.com
 * e-mail  :    blueskyniss@gmail.com
 *//***********************************************************************//**
 * @file
 * @author      nenad
 * @brief       Short desciption of file
 * @addtogroup  module_impl
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
 *          confusing situation why it failed.
 */
PORT_C_NORETURN void esDbgAssert(
    const char *    fnName,
    const char *    expr,
    enum esDbgMsg  msg) {

    const char * assertText;

    PORT_INT_DISABLE();
    switch (msg) {

        case ES_DBG_ARG_OUT_OF_RANGE : {
            assertText = "Argument is out valid range";
            break;
        }

        case ES_DBG_ARG_NOT_VALID : {
            assertText = "Argument is not valid";
            break;
        }

        case ES_DBG_ARG_NULL : {
            assertText = "Argument is NULL pointer";
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
