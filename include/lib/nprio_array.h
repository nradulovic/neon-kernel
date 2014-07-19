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
 * @brief       Priority array header
 * @defgroup    priority_array Priority array
 * @brief       Priority array
 *********************************************************************//** @{ */
/**@defgroup    priority_array_intf Interface
 * @brief       Public interface
 * @{ *//*--------------------------------------------------------------------*/

#ifndef NPRIO_ARRAY_H
#define NPRIO_ARRAY_H

/*=========================================================  INCLUDE FILES  ==*/

#include <stdbool.h>

#include "plat/compiler.h"
#include "arch/cpu.h"
#include "kernel/nub_config.h"
#include "lib/nbitop.h"
#include "lib/nprio_list.h"

/*===============================================================  MACRO's  ==*/

#define NPRIO_ARRAY_BUCKET_BITS                                                 \
    NLOG2_8(NDIVISION_ROUNDUP(CONFIG_PRIORITY_LEVELS, CONFIG_PRIORITY_BUCKETS))

/*------------------------------------------------------  C++ extern begin  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/

/**@brief       Priority array structure
 * @details     An priority array consists of an array of sub-queues. There is
 *              one sub-queue_node per priority level. Each sub-queue contains
 *              the runnable threads at the corresponding priority level. There
 *              is also a bitmap corresponding to the array that is used to
 *              determine effectively the highest-priority task on the queue.
 * @api
 */
struct nprio_array
{
#if (CONFIG_PRIORITY_BUCKETS != 1)
    /**@brief       Priority Bit Map structure
     * @notapi
     */
    struct nprio_bitmap
    {
#if   (CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH) || defined(__DOXYGEN__)
        n_native                     bitGroup;                                  /**<@brief Bit group indicator        */
#endif  /* (CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH) */
        n_native                     bit[NDIVISION_ROUNDUP(
                                         CONFIG_PRIORITY_BUCKETS,
                                         NCPU_DATA_WIDTH)];
                                                                                /**<@brief Bucket indicator           */
    }                           bitmap;                                         /**<@brief Priority bitmap            */
#endif  /* (CONFIG_PRIORITY_BUCKETS != 1) */
    struct nprio_list              sentinel[CONFIG_PRIORITY_BUCKETS];
};

/**@brief       Priority queue_node type
 * @api
 */
typedef struct nprio_array nprio_array;

/*======================================================  GLOBAL VARIABLES  ==*/
/*===================================================  FUNCTION PROTOTYPES  ==*/

#if (CONFIG_PRIORITY_BUCKETS != 1)


static PORT_C_INLINE void nbitmap_init(
    struct nprio_bitmap *       bitmap)
{
#if   (CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH)
    uint_fast8_t                group;

    bitmap->bitGroup = 0u;
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
    uint_fast8_t                group_index;
    uint_fast8_t                bit_index;

    bit_index   = priority & (~((uint_fast8_t)0u) >>
                    (sizeof(priority) * 8u - NLOG2_8(NCPU_DATA_WIDTH)));
    group_index = priority >> NLOG2_8(NCPU_DATA_WIDTH);
    bitmap->bitGroup         |= NCPU_EXP2(group_index);
    bitmap->bit[group_index] |= NCPU_EXP2(bit_index);
#else   /*  (CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH) */
    bitmap->bit[0] |= NCPU_EXP2(priority);
#endif  /* !(CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH) */
}



static PORT_C_INLINE void nbitmap_clear(
    struct nprio_bitmap *       bitmap,
    uint_fast8_t                priority)
{
#if   (CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH)
    uint_fast8_t                group_index;
    uint_fast8_t                bit_index;

    bit_index   = priority & (~((uint_fast8_t)0u) >>
                    (sizeof(priority) * 8u - NLOG2_8(NCPU_DATA_WIDTH)));
    group_index = priority >> NLOG2_8(NCPU_DATA_WIDTH);
    bitmap->bit[group_index] &= ~NCPU_EXP2(bit_index);

    if (bitmap->bit[group_index] == 0u) {                                       /* If this is the last bit cleared in */
        bitmap->bitGroup &= ~NCPU_EXP2(group_index);                            /* this array_entry then clear bit    */
    }                                                                           /* group indicator, too.              */
#else   /*  (CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH) */
    bitmap->bit[0] &= ~NCPU_EXP2(priority);
#endif  /* !(CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH) */
}



static PORT_C_INLINE uint_fast8_t nbitmap_get_highest(
    const struct nprio_bitmap * bitmap)
{
#if   (CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH)
    uint_fast8_t                group_index;
    uint_fast8_t                bit_index;

    group_index = NCPU_FIND_LAST_SET(bitmap->bitGroup);
    bit_index   = NCPU_FIND_LAST_SET(bitmap->bit[group_index]);

    return ((group_index << NLOG2_8(NCPU_DATA_WIDTH)) | bit_index);
#else   /*  (CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH) */
    uint_fast8_t                bit_index;

    bit_index = NCPU_FIND_LAST_SET(bitmap->bit[0]);

    return (bit_index);
#endif  /* !(CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH) */
}



static PORT_C_INLINE bool nbitmap_is_empty(
    const struct nprio_bitmap * bitmap)
{
#if   (CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH)
    if (bitmap->bitGroup == 0u) {
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

static PORT_C_INLINE void nprio_array_init(
    struct nprio_array *        array)
{
    uint_fast8_t                count;

#if (CONFIG_PRIORITY_BUCKETS != 1)
    nbitmap_init(&array->bitmap);
#endif
    count = NARRAY_DIMENSION(array->sentinel);

    while (count-- != 0u) {                                                     /* Initialize each list entry.        */
        nprio_list_init(&array->sentinel[count]);
    }
}



static PORT_C_INLINE void nprio_array_insert(
    struct nprio_array *        array,
    struct nprio_list_node *    node)
{
    uint_fast8_t                bucket;

#if (CONFIG_PRIORITY_BUCKETS != 1)
    bucket = nprio_list_priority(node) >> NPRIO_ARRAY_BUCKET_BITS;
#else
    bucket = 0u;
#endif
#if (CONFIG_PRIORITY_BUCKETS != 1)

    if (nprio_list_is_empty(&array->sentinel[bucket])) {                        /* If adding the first entry in list. */
        nbitmap_set(&array->bitmap, bucket);                                    /* Mark the bucket list as used.      */
    }
#endif
#if (CONFIG_PRIORITY_BUCKETS != CONFIG_PRIORITY_LEVELS)
    nprio_dlist_prio_insert(&array->sentinel[bucket], node);                    /* Priority search and insertion.     */
#else
    nprio_list_fifo_insert(&array->sentinel[bucket], node);                     /* FIFO insertion.                    */
#endif
}



static PORT_C_INLINE void nprio_array_remove(
    struct nprio_array *        array,
    struct nprio_list_node *    node)
{
#if (CONFIG_PRIORITY_BUCKETS != 1)
    uint_fast8_t                bucket;

    bucket = nprio_list_priority(node) >> NPRIO_ARRAY_BUCKET_BITS;
#else
    (void)array;
#endif
    nprio_list_remove(node);
#if (CONFIG_PRIORITY_BUCKETS != 1)

    if (nprio_list_is_empty(&array->sentinel[bucket])) {                        /* If this was the last node in list. */
        nbitmap_clear(&array->bitmap, bucket);                                  /* Mark the bucket as unused.         */
    }
#endif
}



static PORT_C_INLINE void nprio_array_rotate(
    struct nprio_array *        array,
    struct nprio_list_node *    node)
{
    uint_fast8_t                bucket;

#if (CONFIG_PRIORITY_BUCKETS != 1)
    bucket = nprio_list_priority(node) >> NPRIO_ARRAY_BUCKET_BITS;
#else
    bucket = 0u;
#endif
    nprio_list_remove(node);                                                    /* Remove node from bucket.           */
#if (CONFIG_PRIORITY_BUCKETS != CONFIG_PRIORITY_LEVELS)
    nprio_list_prio_insert(&array->sentinel[bucket], node);                     /* Insert the thread at new position. */
#else
    nprio_list_fifo_insert(&array->sentinel[bucket], node);                     /* Insert the thread at new position. */
#endif
}



static PORT_C_INLINE struct nprio_list_node * nprio_array_peek(
    const struct nprio_array *  array)
{
    uint_fast8_t                bucket;

#if (CONFIG_PRIORITY_BUCKETS != 1)
    bucket = nbitmap_get_highest(&array->bitmap);
#else
    bucket = 0u;
#endif

    return (nprio_list_tail(&array->sentinel[bucket]));
}



static PORT_C_INLINE bool nprio_array_is_empty(
    const struct nprio_array *  array)
{
    return (nbitmap_is_empty(&array->bitmap));
}

/*--------------------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//** @} *//*********************************************
 * END of nprio_array.h
 ******************************************************************************/
#endif /* NPRIO_ARRAY_H */
