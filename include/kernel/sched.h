/*
 * This file is part of Neon.
 *
 * Copyright (C) 2010 - 2015 Nenad Radulovic
 *
 * Neon is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Neon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Neon.  If not, see <http://www.gnu.org/licenses/>.
 *
 * web site:    http://github.com/nradulovic
 * e-mail  :    nenad.b.radulovic@gmail.com
 *//***********************************************************************//**
 * @file
 * @author      Nenad Radulovic
 * @brief       Scheduler
 * @defgroup    sched Scheduler
 * @brief       Scheduler
 *********************************************************************//** @{ */

#ifndef NEON_EDS_SCHED_H_
#define NEON_EDS_SCHED_H_

/*=========================================================  INCLUDE FILES  ==*/

#include "port/cpu.h"
#include "shared/config.h"
#include "shared/debug.h"
#include "lib/bias_list.h"
#include "lib/list.h"

/*===============================================================  MACRO's  ==*/

/**@brief       Maximum level of priority possible for application thread
 * @api
 */
#define NTHREAD_PRIORITY_MAX            (CONFIG_PRIORITY_LEVELS - 1u)

/**@brief       Minimum level of priority possible for application thread
 * @api
 */
#define NTHREAD_PRIORITY_MIN            (0u)

/*-------------------------------------------------------  C++ extern base  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/

struct nthread_define
{
    const char *                name;
    uint8_t                     priority;
};

struct nthread
{
    struct nbias_list           node;           /**<@brief Priority queue node*/
    ncpu_reg                    ref;            /**<@brief Reference count    */
#if (CONFIG_REGISTRY == 1) || defined(__DOXYGEN__)
    char                        name[CONFIG_REGISTRY_NAME_SIZE];
    struct ndlist               registry_node;
#endif
#if (CONFIG_API_VALIDATION == 1) || defined(__DOXYGEN__)
    ndebug_magic                signature;
#endif
};

/*======================================================  GLOBAL VARIABLES  ==*/
/*===================================================  FUNCTION PROTOTYPES  ==*/


void nsched_init(void);



void nsched_term(void);



void nsched_thread_init(
    struct nthread *            thread,
    const struct nthread_define * define);



void nsched_thread_term(
    struct nthread *            thread);



void nsched_thread_insert_i(
    struct nthread *            thread);



void nsched_thread_remove_i(
    struct nthread *            thread);



struct nthread * nsched_thread_fetch_i(void);



struct nthread * nsched_get_current(void);

/*--------------------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of sched.h
 ******************************************************************************/
#endif /* NEON_EDS_SCHED_H_ */
