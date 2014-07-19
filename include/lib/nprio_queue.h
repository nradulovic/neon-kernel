/*
 * This file is part of Neon RT Kernel.
 *
 * Copyright (C) 2010 - 2014 Nenad Radulovic
 *
 * Neon RT Kernel is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Neon RT Kernel is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Neon RT Kernel.  If not, see <http://www.gnu.org/licenses/>.
 *
 * web site:    http://github.com/nradulovic
 * e-mail  :    nenad.b.radulovic@gmail.com
 *//***********************************************************************//**
 * @file
 * @author  	Nenad Radulovic
 * @brief       Priority queue header
 * @defgroup    priority_queue Priority queue
 * @brief       Priority queue
 *********************************************************************//** @{ */
/**@defgroup    priority_queue_intf Interface
 * @brief       Public interface
 * @{ *//*--------------------------------------------------------------------*/

#ifndef NPRIO_QUEUE_H
#define NPRIO_QUEUE_H

/*=========================================================  INCLUDE FILES  ==*/

#include <stdbool.h>

#include "plat/compiler.h"
#include "arch/cpu.h"
#include "kernel/nub_config.h"
#include "lib/nbitop.h"
#include "lib/nbias_list.h"

/*===============================================================  MACRO's  ==*/

#define NPRIO_ARRAY_BUCKET_BITS                                                 \
    NLOG2_8(NDIVISION_ROUNDUP(CONFIG_PRIORITY_LEVELS, CONFIG_PRIORITY_BUCKETS))

/*------------------------------------------------------  C++ extern begin  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/

/**@brief       Priority queue structure
 * @details     A priority queue consists of an array of sub-queues. There is
 *              one sub-queue per priority level. Each sub-queue contains the
 *              nodes at the corresponding priority level. There is also a
 *              bitmap corresponding to the array that is used to determine
 *              effectively the highest priority node on the queue.
 * @api
 */
struct nprio_queue
{
#if (CONFIG_PRIORITY_BUCKETS != 1)
    /**@brief       Priority bitmap structure
     * @notapi
     */
    struct nprio_bitmap
    {
#if   (CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH) || defined(__DOXYGEN__)
        n_native                     group;                                     /**<@brief Bit group indicator        */
#endif  /* (CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH) */
        n_native                     bit[NDIVISION_ROUNDUP(
                                         CONFIG_PRIORITY_BUCKETS,
                                         NCPU_DATA_WIDTH)];                     /**<@brief Bucket indicator           */
    }                           bitmap;                                         /**<@brief Priority bitmap            */
#endif  /* (CONFIG_PRIORITY_BUCKETS != 1) */
    struct nbias_list *         sentinel[CONFIG_PRIORITY_BUCKETS];
};

/*======================================================  GLOBAL VARIABLES  ==*/
/*===================================================  FUNCTION PROTOTYPES  ==*/

#if (CONFIG_PRIORITY_BUCKETS != 1)


static PORT_C_INLINE void nbitmap_init(
    struct nprio_bitmap *       bitmap)
{
#if   (CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH)
    uint_fast8_t                group;

    bitmap->group = 0u;
    group = NARRAY_DIMENSION(bitmap->bit);

    while (group-- != 0u) {
        bitmap->bit[group] = 0u;
    }
#else   /*  (CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH) */
    bitmap->bit[0] = 0u;
#endif  /* !(CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH) */
}



static PORT_C_INLINE void nbitmap_set(
    struct nprio_bitmap *       bitmap,
    uint_fast8_t                priority)
{
#if   (CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH)
    uint_fast8_t                group;
    uint_fast8_t                index;

    index = priority &
        ((uint_fast8_t)~0u >> (sizeof(priority) * 8u - NLOG2_8(NCPU_DATA_WIDTH)));
    group = priority >> NLOG2_8(NCPU_DATA_WIDTH);
    bitmap->group      |= NCPU_EXP2(group);
    bitmap->bit[group] |= NCPU_EXP2(index);
#else   /*  (CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH) */
    bitmap->bit[0] |= NCPU_EXP2(priority);
#endif  /* !(CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH) */
}



static PORT_C_INLINE void nbitmap_clear(
    struct nprio_bitmap *       bitmap,
    uint_fast8_t                priority)
{
#if   (CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH)
    uint_fast8_t                group;
    uint_fast8_t                index;

    index = priority &
        ((uint_fast8_t)~0u >> (sizeof(priority) * 8u - NLOG2_8(NCPU_DATA_WIDTH)));
    group = priority >> NLOG2_8(NCPU_DATA_WIDTH);
    bitmap->bit[group] &= ~NCPU_EXP2(index);

    if (bitmap->bit[group] == 0u) {                                             /* If this is the last bit cleared in */
        bitmap->group &= ~NCPU_EXP2(group);                                     /* this array_entry then clear bit    */
    }                                                                           /* group indicator, too.              */
#else   /*  (CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH) */
    bitmap->bit[0] &= ~NCPU_EXP2(priority);
#endif  /* !(CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH) */
}



static PORT_C_INLINE uint_fast8_t nbitmap_get_highest(
    const struct nprio_bitmap * bitmap)
{
#if   (CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH)
    uint_fast8_t                group;
    uint_fast8_t                index;

    group = NCPU_LOG2(bitmap->group);
    index = NCPU_LOG2(bitmap->bit[group]);

    return ((group << NLOG2_8(NCPU_DATA_WIDTH)) | index);
#else   /*  (CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH) */
    uint_fast8_t                index;

    index = NCPU_LOG2(bitmap->bit[0]);

    return (index);
#endif  /* !(CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH) */
}



static PORT_C_INLINE bool nbitmap_is_empty(
    const struct nprio_bitmap * bitmap)
{
#if   (CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH)
    if (bitmap->group == 0u) {
        return (true);
    } else {
        return (false);
    }
#else   /*  (CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH) */
    if (bitmap->bit[0] == 0u) {
        return (true);
    } else {
        return (false);
    }
#endif  /* !(CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH) */
}

#endif  /* (CONFIG_PRIORITY_BUCKETS != 1) */

static PORT_C_INLINE void nprio_queue_init(
    struct nprio_queue *        queue)
{
    uint_fast8_t                count;

#if (CONFIG_PRIORITY_BUCKETS != 1)
    nbitmap_init(&queue->bitmap);
#endif
    count = NARRAY_DIMENSION(queue->sentinel);

    while (count-- != 0u) {                                                     /* Initialize each list entry.        */
        queue->sentinel[count] = NULL;
    }
}



static PORT_C_INLINE void nprio_queue_insert(
    struct nprio_queue *        queue,
    struct nbias_list  *        node)
{
    uint_fast8_t                bucket;

#if (CONFIG_PRIORITY_BUCKETS != 1)
    bucket = nbias_list_get_bias(node) >> NPRIO_ARRAY_BUCKET_BITS;
#else
    bucket = 0u;
#endif

    if (queue->sentinel[bucket] == NULL) {                                      /* If adding the first entry in list. */
        queue->sentinel[bucket] = node;
#if (CONFIG_PRIORITY_BUCKETS != 1)
        nbitmap_set(&queue->bitmap, bucket);                                    /* Mark the bucket list as used.      */
#endif
    } else {
#if (CONFIG_PRIORITY_BUCKETS != CONFIG_PRIORITY_LEVELS)
        nbias_list_sort_insert(queue->sentinel[bucket], node);                  /* Priority search and insertion.     */
#else
        nbias_list_fifo_insert(queue->sentinel[bucket], node);                  /* FIFO insertion.                    */
#endif
    }
}



static PORT_C_INLINE void nprio_queue_remove(
    struct nprio_queue *        queue,
    struct nbias_list  *        node)
{
    uint_fast8_t                bucket;

#if (CONFIG_PRIORITY_BUCKETS != 1)
    bucket = nbias_list_get_bias(node) >> NPRIO_ARRAY_BUCKET_BITS;
#else
    bucket = 0u;
#endif

    if (nbias_list_is_empty(node)) {                                            /* If this was the last node in list. */
        queue->sentinel[bucket] = NULL;
#if (CONFIG_PRIORITY_BUCKETS != 1)
        nbitmap_clear(&queue->bitmap, bucket);                                  /* Mark the bucket as unused.         */
#endif
    } else {
        nbias_list_remove(node);
    }
}



static PORT_C_INLINE void nprio_queue_rotate(
    struct nprio_queue *        queue,
    struct nbias_list  *        node)
{
    uint_fast8_t                bucket;

#if (CONFIG_PRIORITY_BUCKETS != 1)
    bucket = nbias_list_get_bias(node) >> NPRIO_ARRAY_BUCKET_BITS;
#else
    bucket = 0u;
#endif

#if (CONFIG_PRIORITY_BUCKETS != CONFIG_PRIORITY_LEVELS)
    nbias_list_remove(node);                                                    /* Remove node from bucket.           */
    nbias_list_sort_insert(queue->sentinel[bucket], node);                      /* Insert the thread at new position. */
#else
    queue->sentinel[bucket] = nbias_list_next(queue->sentinel[bucket]);
#endif
}



static PORT_C_INLINE struct nbias_list * nprio_queue_peek(
    const struct nprio_queue *  queue)
{
    uint_fast8_t                bucket;

#if (CONFIG_PRIORITY_BUCKETS != 1)
    bucket = nbitmap_get_highest(&queue->bitmap);
#else
    bucket = 0u;
#endif

    return (nbias_list_tail(queue->sentinel[bucket]));
}



static PORT_C_INLINE bool nprio_queue_is_empty(
    const struct nprio_queue *  queue)
{
#if (CONFIG_PRIORITY_BUCKETS != 1)
    return (nbitmap_is_empty(&queue->bitmap));
#else
    if (queue->sentinel[0] == NULL) {
        return (true);
    } else {
        return (false);
    }
#endif
}

/*--------------------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//** @} *//*********************************************
 * END of nprio_queue.h
 ******************************************************************************/
#endif /* NPRIO_QUEUE_H */
