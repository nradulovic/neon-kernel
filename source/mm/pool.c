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
 * @brief       Pool Memory management implementation
 * @addtogroup  mem_pool
 *********************************************************************//** @{ */
/**@defgroup    mem_pool_impl Implementation
 * @brief       Pool Memory management implementation
 * @{ *//*--------------------------------------------------------------------*/

/*=========================================================  INCLUDE FILES  ==*/

#include "base/port/core.h"
#include "base/shared/component.h"
#include "kernel/lib/bitop.h"
#include "kernel/mm/pool.h"

/*=========================================================  LOCAL MACRO's  ==*/

/**@brief       Signature for pool_mem memory manager
 */
#define POOL_MEM_SIGNATURE              ((unsigned int)0xdeadbee2u)

/*======================================================  LOCAL DATA TYPES  ==*/

/**@brief       Pool allocator header structure
 */
struct pool_block
{
    struct pool_block *         next;           /**<@brief Next free block    */
};

/*=============================================  LOCAL FUNCTION PROTOTYPES  ==*/

static void * pool_alloc_i(
    struct nmem *               mem_class,
    size_t                      size);



static void pool_free_i(
    struct nmem *               mem_class,
    void *                      mem);

/*=======================================================  LOCAL VARIABLES  ==*/

static const NCOMPONENT_DEFINE("Pool Memory Module", "Nenad Radulovic");

/*======================================================  GLOBAL VARIABLES  ==*/
/*============================================  LOCAL FUNCTION DEFINITIONS  ==*/


static void * pool_alloc_i(
    struct nmem *               mem_class,
    size_t                      size)
{
    NREQUIRE(NAPI_POINTER, mem_class != NULL);
    NREQUIRE(NAPI_OBJECT,  mem_class->signature == POOL_MEM_SIGNATURE);

    (void)size;

    if (mem_class->base != NULL) {
        struct pool_block *     block;

        block            = mem_class->base;
        mem_class->base  = block->next;
        mem_class->free -= mem_class->size;

        return ((void *)block);
    } else {
        return (NULL);
    }
}




static void pool_free_i(
    struct nmem *               mem_class,
    void *                      mem)
{
    NREQUIRE(NAPI_POINTER, mem_class != NULL);
    NREQUIRE(NAPI_OBJECT,  mem_class->signature == POOL_MEM_SIGNATURE);
    NREQUIRE(NAPI_POINTER, mem != NULL);

    struct pool_block *         block;

    block            = (struct pool_block *)mem;
    block->next      = mem_class->base;
    mem_class->base  = block;
    mem_class->free += mem_class->size;
}

/*===================================  GLOBAL PRIVATE FUNCTION DEFINITIONS  ==*/
/*====================================  GLOBAL PUBLIC FUNCTION DEFINITIONS  ==*/


void npool_init(
    struct npool *              pool,
    void *                      array,
    size_t                      array_size,
    size_t                      block_size)
{
    size_t                      block_cnt;
    size_t                      nblocks;
    struct pool_block *         block;

    NREQUIRE(NAPI_POINTER, pool != NULL);
    NREQUIRE(NAPI_OBJECT,  pool->mem_class.signature != POOL_MEM_SIGNATURE);
    NREQUIRE(NAPI_POINTER, array != NULL);
    NREQUIRE(NAPI_RANGE,   block_size != 0u);
    NREQUIRE(NAPI_RANGE,   block_size <= array_size);

    block_size = NALIGN_UP(block_size, NCPU_DATA_ALIGNMENT);
    nblocks    = array_size / block_size;
    pool->mem_class.base     = array;
    pool->mem_class.size     = block_size;
    pool->mem_class.free     = array_size;
    pool->mem_class.vf_alloc = pool_alloc_i;
    pool->mem_class.vf_free  = pool_free_i;
    block = array;

    for (block_cnt = 0u; block_cnt < nblocks - 1u; block_cnt++) {
        block->next =
            (struct pool_block *)((uint8_t *)block + pool->mem_class.size);
        block = block->next;
    }
    block->next = NULL;
    NOBLIGATION(pool->mem_class.signature = POOL_MEM_SIGNATURE);
}



void * npool_alloc_i(
    struct npool *              pool)
{
    return (pool_alloc_i(&pool->mem_class, 0));
}



void * npool_alloc(
    struct npool *              pool)
{
    ncore_lock                   sys_lock;
    void *                      mem;

    ncore_lock_enter(&sys_lock);
    mem = pool_alloc_i(&pool->mem_class, 0);
    ncore_lock_exit(&sys_lock);

    return (mem);
}



void npool_free_i(
    struct npool *              pool,
    void *                      mem)
{
    pool_free_i(&pool->mem_class, mem);
}



void npool_free(
    struct npool *              pool,
    void *                      mem)
{
    ncore_lock                 sys_lock;

    ncore_lock_enter(&sys_lock);
    pool_free_i(&pool->mem_class, mem);
    ncore_lock_exit(&sys_lock);
}

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//** @} *//*********************************************
 * END of pool_mem.c
 ******************************************************************************/
