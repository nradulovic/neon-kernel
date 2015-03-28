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
 * @brief       Heap Memory management
 * @defgroup    mem_heap Heap Memory management
 * @brief       Heap Memory management
 *********************************************************************//** @{ */

#ifndef NEON_MEM_HEAP_H_
#define NEON_MEM_HEAP_H_

/*=========================================================  INCLUDE FILES  ==*/

#include <stddef.h>

#include "kernel/mm/mem.h"

/*===============================================================  MACRO's  ==*/
/*------------------------------------------------------  C++ extern begin  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/

/**@brief       Heap memory instance structure
 * @details     This structure holds information about dynamic memory instance.
 * @see         nheap_init()
 * @api
 */
struct nheap
{
    struct nmem                 mem_class;
};

/**@brief       Heap memory instance type
 * @api
 */
typedef struct nheap nheap;

/*======================================================  GLOBAL VARIABLES  ==*/
/*===================================================  FUNCTION PROTOTYPES  ==*/


/**@brief       Initialize heap structure instance
 * @param       heap
 *              Pointer to heap structure instance, see @ref nheap.
 * @param       storage
 *              Pointer to reserved memory space. Usually this will be an array
 *              of bytes which are statically alloacated.
 * @param       Size of storage reserved memory in bytes.
 * @details     This function must be called before @c heap structure can be
 *              used by other functions.
 * @api
 */
void nheap_init(
    struct nheap *              heap,
    void *                      storage,
    size_t                      size);



/**@brief       Terminate heap instance
 * @param       heap
 *              Pointer to heap structure instance
 * @api
 */
void nheap_term(
    struct nheap *              heap);



void * nheap_alloc_i(
    struct nheap *              heap,
    size_t                      size);



void * nheap_alloc(
    struct nheap *              heap,
    size_t                      size);



void nheap_free_i(
    struct nheap *              heap,
    void *                      mem);



void nheap_free(
    struct nheap *              heap,
    void *                      mem);

/*--------------------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of heap.h
 ******************************************************************************/
#endif /* NEON_MEM_HEAP_H_ */
