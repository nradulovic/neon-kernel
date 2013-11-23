/*
 * This file is part of eSolid.
 *
 * Copyright (C) 2010 - 2013 Nenad Radulovic
 *
 * eSolid is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * eSolid is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with eSolid.  If not, see <http://www.gnu.org/licenses/>.
 *
 * web site:    http://github.com/nradulovic
 * e-mail  :    nenad.b.radulovic@gmail.com
 *//***********************************************************************//**
 * @file
 * @author      Nenad Radulovic
 * @brief       Implementation of Debug module
 * @addtogroup  dbg_impl
 *********************************************************************//** @{ */

/*=========================================================  INCLUDE FILES  ==*/

#include "arch/compiler.h"
#include "arch/cpu.h"
#include "kernel/dbg.h"

/*=========================================================  LOCAL MACRO's  ==*/
/*======================================================  LOCAL DATA TYPES  ==*/
/*=============================================  LOCAL FUNCTION PROTOTYPES  ==*/
/*=======================================================  LOCAL VARIABLES  ==*/

/**@brief       Definition text of debug messages
 * @note        This array needs to be in synchronization with enum esDbgMsgNum.
 */
static const PORT_C_ROM char * const PORT_C_ROM_VAR DbgMsg[] = {
    "Value is out of valid range.",                                             /* ES_DBG_OUT_OF_RANGE                                      */
    "Object is not valid.",                                                     /* ES_DBG_OBJECT_NOT_VALID                                  */
    "Pointer has NULL value.",                                                  /* ES_DBG_POINTER_NULL                                      */
    "Object/method usage failure.",                                             /* ES_DBG_USAGE_FAILURE                                     */
    "Not enough memory available.",                                             /* ES_DBG_NOT_ENOUGH_MEM                                    */
    "A method is not implemented",                                              /* ES_DBG_NOT_IMPLEMENTED                                   */
    "Unknown error."                                                            /* ES_DBG_UNKNOWN_ERROR                                     */
};

/*======================================================  GLOBAL VARIABLES  ==*/
/*============================================  LOCAL FUNCTION DEFINITIONS  ==*/
/*===================================  GLOBAL PRIVATE FUNCTION DEFINITIONS  ==*/
/*====================================  GLOBAL PUBLIC FUNCTION DEFINITIONS  ==*/

/* 1)       This function will disable all interrupts to prevent any new
 *          interrupts to execute which can trigger another assert causing a
 *          very confusing situation of why it failed.
 */
PORT_C_NORETURN void dbgAssert(
    const PORT_C_ROM struct dbgCobj_ * cObj,
    const PORT_C_ROM char * expr,
    enum esDbgMsgNum    msg) {

    struct esDbgReport  dbgReport;

    PORT_INT_DISABLE();

    if (ES_DBG_UNKNOWN_ERROR > msg) {
        msg = ES_DBG_UNKNOWN_ERROR;
    }
    dbgReport.modName   = cObj->mod->name;
    dbgReport.modDesc   = cObj->mod->desc;
    dbgReport.modAuthor = cObj->mod->auth;
    dbgReport.modFile   = cObj->mod->file;
    dbgReport.fnName    = cObj->fn;
    dbgReport.expr      = expr;
    dbgReport.msgText   = DbgMsg[msg];
    dbgReport.line      = cObj->line;
    dbgReport.msgNum    = msg;
    userAssert(
        &dbgReport);
    PORT_CPU_TERM();

    while (TRUE);
}

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of dbg.c
 ******************************************************************************/
