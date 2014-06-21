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
 * @brief       Priority array implementation
 * @addtogroup  priority_array
 *************************************************************************************************************//** @{ */
/**@defgroup    priority_array_impl Implementation
 * @brief       Implementation
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

/*=================================================================================================  INCLUDE FILES  ==*/

#include "kernel/nprio_array.h"
#include "kernel/nthread.h"
#include "kernel/nlist.h"

/*=================================================================================================  LOCAL MACRO's  ==*/
/*==============================================================================================  LOCAL DATA TYPES  ==*/
/*=====================================================================================  LOCAL FUNCTION PROTOTYPES  ==*/

#if (CONFIG_PRIORITY_BUCKETS != 1)
/**@brief       Initialize bitmap
 * @param       bitmap
 *              Pointer to the bit map structure
 */
static PORT_C_INLINE void bitmap_init(
    struct nprio_bitmap *       bitmap);

/**@brief       Set the bit corresponding to the priority argument
 * @param       bitmap
 *              Pointer to the bit map structure
 * @param       priority
 *              Priority which will be marked as used
 */
static PORT_C_INLINE void bitmap_set(
    struct nprio_bitmap *       bitmap,
    uint_fast8_t                priority);

/**@brief       Clear the bit corresponding to the priority argument
 * @param       bitmap
 *              Pointer to the bit map structure
 * @param       priority
 *              Priority which will be marked as unused
 */
static PORT_C_INLINE void bitmap_clear(
    struct nprio_bitmap *       bitmap,
    uint_fast8_t                priority);

/**@brief       Get the highest priority set
 * @param       bitmap
 *              Pointer to the bit map structure
 * @return      The number of the highest priority marked as used
 */
static PORT_C_INLINE uint_fast8_t bitmap_get_highest(
    const struct nprio_bitmap * bitmap);
#endif /* CONFIG_PRIORITY_BUCKETS != 1 */

/*===============================================================================================  LOCAL VARIABLES  ==*/
/*==============================================================================================  GLOBAL VARIABLES  ==*/
/*====================================================================================  LOCAL FUNCTION DEFINITIONS  ==*/

#if (CONFIG_PRIORITY_BUCKETS != 1)
static PORT_C_INLINE void bitmap_init(
    struct nprio_bitmap *     bitmap)
{
#if   (CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH)
    uint_fast8_t                group;

    bitmap->bitGroup = 0u;
    group = NARRAY_DIMENSION(bitmap->bit);

    while (group-- != 0u)
    {
        bitmap->bit[group] = 0u;
    }
#else
    bitmap->bit[0] = 0u;
#endif
}

static PORT_C_INLINE void bitmap_set(
    struct nprio_bitmap *     bitmap,
    uint_fast8_t                priority)
{
#if   (CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH)
    uint_fast8_t                group_index;
    uint_fast8_t                bit_index;

    bit_index   = priority & (~((uint_fast8_t)0u) >> (sizeof(priority) * 8u - N_LOG2_8(NCPU_DATA_WIDTH)));
    group_index = priority >> N_LOG2_8(NCPU_DATA_WIDTH);
    bitmap->bitGroup         |= NCPU_EXP2(group_index);
    bitmap->bit[group_index] |= NCPU_EXP2(bit_index);
#else
    bitmap->bit[0] |= NCPU_EXP2(priority);
#endif
}

static PORT_C_INLINE void bitmap_clear(
    struct nprio_bitmap *     bitmap,
    uint_fast8_t                priority)
{
#if   (CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH)
    uint_fast8_t                group_index;
    uint_fast8_t                bit_index;

    bit_index   = priority & (~((uint_fast8_t)0u) >> (sizeof(priority) * 8u - N_LOG2_8(NCPU_DATA_WIDTH)));
    group_index = priority >> N_LOG2_8(NCPU_DATA_WIDTH);
    bitmap->bit[group_index] &= ~NCPU_EXP2(bit_index);

    if (bitmap->bit[group_index] == 0u)                                         /* If this is the last bit cleared in */
    {                                                                           /* this list then clear bit group     */
        bitmap->bitGroup &= ~NCPU_EXP2(group_index);                          /* indicator, too.                    */
    }
#else
    bitmap->bit[0] &= ~NCPU_EXP2(priority);
#endif
}

static PORT_C_INLINE uint_fast8_t bitmap_get_highest(
    const struct nprio_bitmap * bitmap)
{
#if   (CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH)
    uint_fast8_t                group_index;
    uint_fast8_t                bit_index;

    group_index = NCPU_FIND_LAST_SET(bitmap->bitGroup);
    bit_index   = NCPU_FIND_LAST_SET(bitmap->bit[group_index]);

    return ((group_index << N_LOG2_8(NCPU_DATA_WIDTH)) | bit_index);
#else
    uint_fast8_t                bit_index;

    bit_index = NCPU_FIND_LAST_SET(bitmap->bit[0]);

    return (bit_index);
#endif
}
#endif /* CONFIG_PRIORITY_BUCKETS != 1 */

/*===========================================================================  GLOBAL PRIVATE FUNCTION DEFINITIONS  ==*/
/*============================================================================  GLOBAL PUBLIC FUNCTION DEFINITIONS  ==*/

void nprio_array_init(
    struct nprio_array *        array)
{
    uint_fast8_t                count;

#if (CONFIG_PRIORITY_BUCKETS != 1)
    bitmap_init(&array->bitmap);
#endif
    count = NARRAY_DIMENSION(array->sentinel);

    while (count-- != 0u)
    {
        array->sentinel[count] = NULL;
    }
}

#define BUCKET_BITS                         N_LOG2_8(NDIVISION_ROUNDUP(CONFIG_PRIORITY_LEVELS, CONFIG_PRIORITY_BUCKETS))

void nprio_array_insert(
    struct nprio_array *        array,
    struct nthread *            thread)
{
    uint_fast8_t                bucket;

#if (CONFIG_PRIORITY_BUCKETS != 1)
    bucket = thread->priority >> BUCKET_BITS;
#else
    bucket = 0u;
#endif

    if (array->sentinel[bucket] == NULL)
    {
        array->sentinel[bucket] = thread;
#if (CONFIG_PRIORITY_BUCKETS != 1)
        bitmap_set(&array->bitmap, bucket);                           /* Mark the bucket list as used     */
#endif
    }
    else
    {
        struct nthread *        entry;

        entry = array->sentinel[bucket];
#if (CONFIG_PRIORITY_BUCKETS != CONFIG_PRIORITY_LEVELS)
        do
        {
            if (entry->priority < thread->priority)
            {
                break;
            }
            entry = NDLIST_NEXT(array_entry.list, entry);
        }
        while (entry != array->sentinel[bucket]);

        if (array->sentinel[bucket]->priority < thread->priority)
        {
            array->sentinel[bucket] = thread;
        }
#endif
        NDLIST_ADD_AFTER(array_entry.list, entry, thread);
    }
}

void nprio_array_remove(
    struct nprio_array *        array,
    struct nthread *            thread)
{
    uint_fast8_t                bucket;

#if (CONFIG_PRIORITY_BUCKETS != 1)
    bucket = thread->priority >> BUCKET_BITS;
#else
    bucket = 0u;
#endif

    if (NDLIST_IS_EMPTY(array_entry.list, thread))
    {
        array->sentinel[bucket] = NULL;                                         /* Remove the mark since this list is */
                                                                                /* used.                              */
#if (CONFIG_PRIORITY_BUCKETS != 1)
        bitmap_clear(&array->bitmap, bucket);
#endif
    }
    else
    {
        if (array->sentinel[bucket] == thread)
        {
            array->sentinel[bucket] = NDLIST_NEXT(array_entry.list, thread);
        }
        NDLIST_REMOVE(array_entry.list, thread);
    }
}

struct nthread * nprio_array_peek(
    const struct nprio_array *  array)
{
    uint_fast8_t                bucket;

#if (CONFIG_PRIORITY_BUCKETS != 1)
    bucket = bitmap_get_highest(&array->bitmap);
#else
    bucket = 0u;
#endif

    return (array->sentinel[bucket]);
}

struct nthread * nprio_array_rotate_thread(
    struct nprio_array *        array,
    struct nthread *            thread)
{
    uint_fast8_t                bucket;

#if (CONFIG_PRIORITY_BUCKETS != 1)
    bucket = thread->priority >> BUCKET_BITS;
#else
    bucket = 0u;
#endif

#if (CONFIG_PRIORITY_BUCKETS != CONFIG_PRIORITY_LEVELS)
    {
        struct nthread *        entry;

        entry = array->sentinel[bucket] = NDLIST_NEXT(array_entry.list, thread);
        NDLIST_REMOVE(array_entry.list, thread);
        do
        {
            if (entry->priority < thread->priority)
            {
                break;
            }
            entry = NDLIST_NEXT(array_entry.list, entry);
        }
        while (entry != array->sentinel[bucket]);

        if (array->sentinel[bucket]->priority < thread->priority)
        {
            array->sentinel[bucket] = thread;
        }
        NDLIST_ADD_AFTER(array_entry.list, entry, thread);
    }
#else
    array->sentinel[bucket] = NDLIST_NEXT(array_entry.list, array->sentinel[bucket]);
#endif

    return (array->sentinel[bucket]);
}

void nprio_array_init_entry(
    struct nthread *            thread)
{
    NDLIST_INIT(array_entry.list, thread);
}

/*========================================================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//** @} *//*************************************************************************************
 * END of nthread_queue.c
 **********************************************************************************************************************/
