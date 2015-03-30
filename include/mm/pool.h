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
 * @brief       Pool Memory management
 * @defgroup    mem_pool Pool Memory management
 * @brief       Pool Memory management
 *********************************************************************//** @{ */

#ifndef NEON_MEM_POOL_H_
#define NEON_MEM_POOL_H_

/*=========================================================  INCLUDE FILES  ==*/

#include <stddef.h>

#include "shared/bitop.h"
#include "mm/mem.h"

/*===============================================================  MACRO's  ==*/

#define NPOOL_MEM_COMPUTE_SIZE(blocks, blockSize)                               \
    ((blocks) * (NALIGN_UP(blockSize, sizeof(ncpu_reg))))

/*------------------------------------------------------  C++ extern begin  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/

/**@brief       Pool memory instance
 * @details     This structure holds information about pool_mem memory instance.
 * @p           This structure hold information about pool_mem and block sizes.
 *              Additionally, it holds a guard member which will ensure mutual
 *              exclusion in preemption environments.
 * @see         npool_init()
 * @api
 */
struct npool
{
    struct nmem                 mem_class;
};

/**@brief       Pool memory instance pool_mem type
 * @api
 */
typedef struct npool npool;

/*======================================================  GLOBAL VARIABLES  ==*/
/*===================================================  FUNCTION PROTOTYPES  ==*/


/**@brief       Initializes pool_mem memory instance
 * @param       pool_mem
 *              Pointer to pool_mem memory instance, see @ref npool.
 * @param       pool_mem
 *              Reserved memory area for pool_mem allocator.
 * @param       array_size
 *              The get_size of reserved memory area expressed in bytes.
 * @param       block_size
 *              The get_size of one block expressed in bytes.
 * @details     This function must be called before any call to npool_alloc_i()
 *              or npool_alloc().
 * @api
 */
void npool_init(
    struct npool *              pool,
    void *                      array,
    size_t                      array_size,
    size_t                      block_size);



/**@brief       Allocate one block from memory pool_mem
 * @param       pool_mem
 *              Pointer to pool_mem memory instance, see @ref npool.
 * @return      eSolid standard error:
 *              - @ref ES_ERROR_NONE - no error occurred
 *              - @ref ES_ERROR_NO_MEMORY - not enough memory available
 * @iclass
 */
void * npool_alloc_i(
    struct npool *              pool);



/**@brief       Allocate one block from memory pool_mem
 * @param       pool_mem
 *              Pointer to pool_mem memory instance, see @ref npool.
 * @return      eSolid standard error:
 *              - @ref ES_ERROR_NONE - no error occurred
 *              - @ref ES_ERROR_NO_MEMORY - not enough memory available
 * @api
 */
void * npool_alloc(
    struct npool *              pool);



/**
 * @brief       Oslobadja prethodno alocirani blok
 * @param       [in] pool_mem             Deskriptor pool_mem alokatora
 * @param       [in] mem                Prethodno alociran blok memorije
 * @iclass
 */
void npool_free_i(
    struct npool *              pool,
    void *                      mem);



/**
 * @brief       Oslobadja prethodno alocirani blok
 * @param       [in] pool_mem             Deskriptor pool_mem alokatora
 * @param       [in] mem                Prethodno alociran blok memorije
 * @note        Funkcija koristi makroe @ref ES_LOCK_SYS i
 *              @ref ES_UNLOCK_SYS za zastitu memorije od istovremenog
 *              pristupa.
 * @api
 */
void npool_free(
    struct npool *              pool,
    void *                      mem);

/*--------------------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of pool.h
 ******************************************************************************/
#endif /* NEON_MEM_POOL_H_ */
