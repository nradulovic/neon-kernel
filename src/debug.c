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
 *//***************************************************************************************************************//**
 * @file
 * @author      Nenad Radulovic
 * @brief       Debug support Implementation
 * @addtogroup  nano_dbg
 *********************************************************************//** @{ */
/**@defgroup    nano_dbg_impl Debug support Implementation
 * @brief       Debug support Implementation
 * @{ *//*--------------------------------------------------------------------*/

/*=========================================================  INCLUDE FILES  ==*/

#include <stdbool.h>
#include <stddef.h>

#include "plat/compiler.h"
#include "arch/cpu.h"
#include "arch/intr.h"

#include "kernel/ndebug.h"

/*=========================================================  LOCAL MACRO's  ==*/
/*======================================================  LOCAL DATA TYPES  ==*/
/*=============================================  LOCAL FUNCTION PROTOTYPES  ==*/
/*=======================================================  LOCAL VARIABLES  ==*/
/*======================================================  GLOBAL VARIABLES  ==*/
/*============================================  LOCAL FUNCTION DEFINITIONS  ==*/
/*===================================  GLOBAL PRIVATE FUNCTION DEFINITIONS  ==*/
/*====================================  GLOBAL PUBLIC FUNCTION DEFINITIONS  ==*/

/* 1)       This function will disable all interrupts to prevent any new
 *          interrupts to execute which can trigger another assert causing a
 *          very confusing situation of why it failed.
 */
PORT_C_NORETURN void ndebug_assert(
    const PORT_C_ROM struct ndebug_object * object,
    const PORT_C_ROM char *                 expression,
    const PORT_C_ROM char *                 message)
{
    struct ndebug_report report;

    ES_INTR_DISABLE();

    if (object->mod != NULL)
    {
        report.mod_name   = object->mod->name;
        report.mod_desc   = object->mod->desc;
        report.mod_author = object->mod->auth;
        report.mod_file   = object->mod->file;
    }
    else
    {
        report.mod_name   = "Unnamed";
        report.mod_desc   = "not specified";
        report.mod_author = "not specified";
        report.mod_file   = "not specified";
    }
    report.fn_name = object->fn;
    report.expr    = expression;
    report.msg     = message;
    report.line    = object->line;
    userAssert(&report);
    ES_CPU_TERM();

    while (true);
}

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//** @} *//*********************************************
 * END of dbg.c
 ******************************************************************************/
