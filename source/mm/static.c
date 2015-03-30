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
 * @brief       Static Memory management implementation
 * @addtogroup  mem_static
 *********************************************************************//** @{ */
/**@defgroup    mem_static_impl Implementation
 * @brief       Static Memory management implementation
 * @{ *//*--------------------------------------------------------------------*/

/*=========================================================  INCLUDE FILES  ==*/

#include "port/core.h"
#include "shared/component.h"
#include "shared/bitop.h"
#include "mm/static.h"

/*=========================================================  LOCAL MACRO's  ==*/

/**@brief       Signature for static memory manager
 */
#define STATIC_MEM_SIGNATURE            ((ncpu_reg)0xdeadbee0u)

/*======================================================  LOCAL DATA TYPES  ==*/
/*=============================================  LOCAL FUNCTION PROTOTYPES  ==*/


static void * static_alloc_i(
    struct nmem *               mem_class,
    size_t                      size);



static void static_free_i(
    struct nmem *               mem_class,
    void *                      mem);

/*=======================================================  LOCAL VARIABLES  ==*/

static const NCOMPONENT_DEFINE("Static Memory Management", "Nenad Radulovic");

/*======================================================  GLOBAL VARIABLES  ==*/
/*============================================  LOCAL FUNCTION DEFINITIONS  ==*/


static void * static_alloc_i(
    struct nmem *               mem_class,
    size_t                      size)
{
    NREQUIRE(NAPI_POINTER, mem_class != NULL);
    NREQUIRE(NAPI_POINTER, mem_class->signature == STATIC_MEM_SIGNATURE);

    size = NALIGN_UP(size, NCPU_DATA_ALIGNMENT);

    if (size <= mem_class->free) {
        mem_class->free -= size;

        return ((void *)&((uint8_t *)mem_class->base)[mem_class->free]);
    } else {

        return (NULL);
    }
}

static void static_free_i(
    struct nmem *               mem_class,
    void *                      mem)
{
    (void)mem;
    (void)mem_class;

    NASSERT_ALWAYS("illegal static memory call");
}

/*===================================  GLOBAL PRIVATE FUNCTION DEFINITIONS  ==*/
/*====================================  GLOBAL PUBLIC FUNCTION DEFINITIONS  ==*/


void nstatic_init(
    struct nstatic *            static_mem,
    void *                      storage,
    size_t                      size)
{
    NREQUIRE(NAPI_POINTER, static_mem != NULL);
    NREQUIRE(NAPI_POINTER, storage != NULL);
    NREQUIRE(NAPI_RANGE,   size > NCPU_DATA_ALIGNMENT);

    static_mem->mem_class.base     = storage;
    static_mem->mem_class.size     = NALIGN(size, NCPU_DATA_ALIGNMENT);
    static_mem->mem_class.free     = NALIGN(size, NCPU_DATA_ALIGNMENT);
    static_mem->mem_class.vf_alloc = static_alloc_i;
    static_mem->mem_class.vf_free  = static_free_i;

    NOBLIGATION(static_mem->mem_class.signature = STATIC_MEM_SIGNATURE);
}



void * nstatic_alloc_i(
    struct nstatic *            static_mem,
    size_t                      size)
{
    return (static_alloc_i(&static_mem->mem_class, size));
}



void * nstatic_alloc(
    struct nstatic *            static_mem,
    size_t                      size)
{
    ncore_lock                   sys_lock;
    void *                      mem;

    ncore_lock_enter(&sys_lock);
    mem = static_alloc_i(&static_mem->mem_class, size);
    ncore_lock_exit(&sys_lock);

    return (mem);
}

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//** @} *//*********************************************
 * END of static_mem.c
 ******************************************************************************/
