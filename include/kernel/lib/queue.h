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
 * @brief       Generic queue header
 * @defgroup    generic_queue Generic queue
 * @brief       Generic queue
 *********************************************************************//** @{ */

#ifndef NEON_LIB_QUEUE_H_
#define NEON_LIB_QUEUE_H_

/*=========================================================  INCLUDE FILES  ==*/

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "base/port/compiler.h"
#include "base/port/core.h"

/*===============================================================  MACRO's  ==*/

#define NQUEUE_SIZEOF(elements)             (sizeof(void * [1]) * (elements))

/*-------------------------------------------------------  C++ extern base  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/

/**@brief       Queue structure
 * @api
 */
struct nqueue
{
    void **                     storage;
    ncpu_reg                    head;
    ncpu_reg                    tail;
    ncpu_reg                    free;
    ncpu_reg                    size;
};

typedef struct nqueue nqueue;

/*======================================================  GLOBAL VARIABLES  ==*/
/*===================================================  FUNCTION PROTOTYPES  ==*/


PORT_C_INLINE 
void nqueue_init(
    struct nqueue *             queue,
    void *                      storage,
    size_t                      size)
{
    queue->storage = storage;
    queue->head    = 0u;
    queue->tail    = 0u;
    queue->free    = (ncpu_reg)size;
    queue->size    = (ncpu_reg)size;
}



PORT_C_INLINE 
void nqueue_term(
    struct nqueue *             queue)
{
    queue->head    = 0u;
    queue->tail    = 0u;
    queue->free   = 0u;
    queue->size    = 0u;
}



PORT_C_INLINE 
void nqueue_put_fifo(
    struct nqueue *             queue,
    void *                      item)
{
    queue->storage[queue->head++] = item;

    if (queue->head == queue->size) {
        queue->head = 0u;
    }
    queue->free--;
}



PORT_C_INLINE 
void nqueue_put_lifo(
    struct nqueue *             queue,
    void *                      item)
{
    if (queue->tail == 0u) {
        queue->tail = queue->size;
    }
    queue->storage[--queue->tail] = item;
    queue->free--;
}



PORT_C_INLINE
void * nqueue_get(
    struct nqueue *             queue)
{
    void *                      tmp;

    tmp = queue->storage[queue->tail++];

    if (queue->tail == queue->size) {
        queue->tail = 0u;
    }
    queue->free++;

    return (tmp);
}



PORT_C_INLINE 
size_t nqueue_size(
    const struct nqueue *       queue)
{
    return ((size_t)queue->size);
}



PORT_C_INLINE 
size_t nqueue_free(
    const struct nqueue *       queue)
{
    return ((size_t)queue->free);
}



PORT_C_INLINE 
void * nqueue_storage(
    const struct nqueue *       queue)
{
    return (queue->storage);
}



PORT_C_INLINE 
bool nqueue_is_full(
    const struct nqueue *       queue)
{
    if (queue->free) {
        return (false);
    } else {
        return (true);
    }
}



PORT_C_INLINE 
bool nqueue_is_empty(
    const struct nqueue *       queue)
{
    if (queue->free == queue->size) {
        return (true);
    } else {
        return (false);
    }
}

/*--------------------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of queue.h
 ******************************************************************************/
#endif /* NEON_LIB_QUEUE_H_ */

