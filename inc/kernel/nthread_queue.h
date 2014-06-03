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
 * @author  	Nenad Radulovic
 * @brief       Priority queue header
 * @defgroup    priority_queue Priority queue
 * @brief       Priority queue
 *************************************************************************************************************//** @{ */
/**@defgroup    priority_queue_intf Interface
 * @brief       Public interface
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

#ifndef NTHREAD_QUEUE_H_
#define NTHREAD_QUEUE_H_

/*=================================================================================================  INCLUDE FILES  ==*/

#include <stdint.h>

#include "plat/compiler.h"
#include "arch/cpu.h"

#include "kernel/nkernel_config.h"

#include "kernel/nbitop.h"

/*=======================================================================================================  MACRO's  ==*/
/*----------------------------------------------------------------------------------------------  C++ extern begin  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*====================================================================================================  DATA TYPES  ==*/

/**@brief       Priority Bit Map structure
 * @notapi
 */
struct nthread_bitmap
{
#if   (CONFIG_PRIORITY_LEVELS > ES_CPU_DEF_DATA_WIDTH) || defined(__DOXYGEN__)
    natomic                     bitGroup;                                       /**<@brief Bit list indicator         */
#endif
    /**@brief       Bit priority indicator
     */
    natomic                     bit[NDIVISION_ROUNDUP(CONFIG_PRIORITY_LEVELS, ES_CPU_DEF_DATA_WIDTH)];
};

struct nthread_list
{
    struct nthread *            next;
    struct nthread *            prev;
};

struct nthread_queue_entry
{
    struct nthread_queue *      container;
    struct nthread_list         list;
};

/**@brief       Priority queue structure
 * @api
 */
struct nthread_queue
{
    struct nthread_bitmap       bitmap;                                         /**<@brief Priority bitmap            */
    struct nthread *            sentinel[CONFIG_PRIORITY_LEVELS];
};

/**@brief       Priority queue type
 * @api
 */
typedef struct nthread_queue nthread_pqueue;

/*==============================================================================================  GLOBAL VARIABLES  ==*/
/*===========================================================================================  FUNCTION PROTOTYPES  ==*/

void nthread_queue_init(
    struct nthread_queue *      queue);

void nthread_queue_insert(
    struct nthread_queue *      queue,
    struct nthread *            thread);

void nthread_queue_remove(
    struct nthread *            thread);

struct nthread * nthread_queue_peek(
    const struct nthread_queue * queue);

struct nthread * nthread_queue_rotate(
    struct nthread_queue *      queue);

void nthread_queue_init_entry(
    struct nthread *            thread);

uint_fast8_t nthread_queue_get_level(
    const struct nthread_queue * queue);

struct nthread_queue * nthread_queue_get_container(
    const struct nthread *      thread);

/*------------------------------------------------------------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*========================================================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//** @} *//*************************************************************************************
 * END of nthread_queue.h
 **********************************************************************************************************************/
#endif /* NTHREAD_QUEUE_H_ */
