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

/**@brief       Is bit map empty?
 * @param       bitmap
 *              Pointer to the bit map structure
 * @return      The status of the bit map
 *  @retval     true  - there is at least one thread in bitmap
 *  @retval     false - there is no thread in bitmap
 */
static PORT_C_INLINE uint_fast8_t bitmap_get_level(
    const struct nprio_bitmap * bitmap);

/*===============================================================================================  LOCAL VARIABLES  ==*/
/*==============================================================================================  GLOBAL VARIABLES  ==*/
/*====================================================================================  LOCAL FUNCTION DEFINITIONS  ==*/

static PORT_C_INLINE void bitmap_init(
    struct nprio_bitmap *     bitmap)
{
#if   (CONFIG_PRIORITY_LEVELS > NCPU_DATA_WIDTH)
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
#if   (CONFIG_PRIORITY_LEVELS > NCPU_DATA_WIDTH)
    uint_fast8_t                group_index;
    uint_fast8_t                bit_index;

    bit_index   = priority & (~((uint_fast8_t)0u) >> (sizeof(priority) * 8u - ES_UINT8_LOG2(NCPU_DATA_WIDTH)));
    group_index = priority >> ES_UINT8_LOG2(NCPU_DATA_WIDTH);
    bitmap->bitGroup         |= NCPU_POWER2(group_index);
    bitmap->bit[group_index] |= NCPU_POWER2(bit_index);
#else
    bitmap->bit[0] |= NCPU_POWER2(priority);
#endif
}

static PORT_C_INLINE void bitmap_clear(
    struct nprio_bitmap *     bitmap,
    uint_fast8_t                priority)
{
#if   (CONFIG_PRIORITY_LEVELS > NCPU_DATA_WIDTH)
    uint_fast8_t                group_index;
    uint_fast8_t                bit_index;

    bit_index   = priority & (~((uint_fast8_t)0u) >> (sizeof(priority) * 8u - ES_UINT8_LOG2(NCPU_DATA_WIDTH)));
    group_index = priority >> ES_UINT8_LOG2(NCPU_DATA_WIDTH);
    bitmap->bit[group_index] &= ~NCPU_POWER2(bit_index);

    if (bitmap->bit[group_index] == 0u)                                         /* If this is the last bit cleared in */
    {                                                                           /* this list then clear bit group     */
        bitmap->bitGroup &= ~NCPU_POWER2(group_index);                          /* indicator, too.                    */
    }
#else
    bitmap->bit[0] &= ~NCPU_POWER2(priority);
#endif
}

static PORT_C_INLINE uint_fast8_t bitmap_get_highest(
    const struct nprio_bitmap * bitmap)
{
#if   (CONFIG_PRIORITY_LEVELS > NCPU_DATA_WIDTH)
    uint_fast8_t                group_index;
    uint_fast8_t                bit_index;

    group_index = NCPU_FIND_LAST_SET(bitmap->bitGroup);
    bit_index   = NCPU_FIND_LAST_SET(bitmap->bit[group_index]);

    return ((group_index << ES_UINT8_LOG2(NCPU_DATA_WIDTH)) | bit_index);
#else
    uint_fast8_t                bit_index;

    bit_index = NCPU_FIND_LAST_SET(bitmap->bit[0]);

    return (bit_index);
#endif
}

static PORT_C_INLINE uint_fast8_t bitmap_get_level(
    const struct nprio_bitmap * bitmap)
{
#if   (CONFIG_PRIORITY_LEVELS > NCPU_DATA_WIDTH)

    return (bitmap->bitGroup);
#else

    return (bitmap->bit[0]);
#endif
}

/*===========================================================================  GLOBAL PRIVATE FUNCTION DEFINITIONS  ==*/
/*============================================================================  GLOBAL PUBLIC FUNCTION DEFINITIONS  ==*/

void nprio_array_init(
    struct nprio_array *        array)
{
    uint_fast8_t                count;

    bitmap_init(&array->bitmap);
    count = NARRAY_DIMENSION(array->sentinel);

    while (count-- != 0u)
    {
        array->sentinel[count] = NULL;
    }
}

void nprio_array_insert(
    struct nprio_array *        array,
    struct nthread *            thread)
{
    thread->array_entry.container = array;

    if (array->sentinel[thread->priority] == NULL)
    {
        array->sentinel[thread->priority] = thread;
        bitmap_set(&array->bitmap, thread->priority);                           /* Mark the priority list as used     */
    }
    else
    {
        NDLIST_ADD_HEAD(array_entry.list, array->sentinel[thread->priority], thread);
    }
}

void nprio_array_remove(
    struct nthread *            thread)
{
    struct nprio_array *        array;

    array = thread->array_entry.container;

    if (NDLIST_IS_EMPTY(array_entry.list, thread))
    {
        array->sentinel[thread->priority] = NULL;                               /* Remove the mark since this list is */
        bitmap_clear(&array->bitmap, thread->priority);                         /* used.                              */
    }
    else
    {
        if (array->sentinel[thread->priority] == thread)
        {
            array->sentinel[thread->priority] = NDLIST_NEXT(array_entry.list, thread);
        }
        NDLIST_REMOVE(array_entry.list, thread);
    }
    thread->array_entry.container = NULL;
}

struct nthread * nprio_array_peek(
    const struct nprio_array *  array)
{
    uint_fast8_t                priority;

    priority = bitmap_get_highest(&array->bitmap);

    return (array->sentinel[priority]);
}

struct nthread * nprio_array_rotate_level(
    struct nprio_array *        array,
    uint_fast8_t                level)
{
    array->sentinel[level] = NDLIST_NEXT(array_entry.list, array->sentinel[level]);

    return (array->sentinel[level]);
}

void nprio_array_init_entry(
    struct nthread *            thread)
{
    thread->array_entry.container = NULL;
    NDLIST_INIT(array_entry.list, thread);
}

uint_fast8_t nprio_array_get_level(
    const struct nprio_array *  array)
{
    return (bitmap_get_level(&array->bitmap));
}

struct nprio_array * nprio_array_get_container(
    const struct nthread *      thread)
{
    return (thread->array_entry.container);
}

/*========================================================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//** @} *//*************************************************************************************
 * END of nthread_queue.c
 **********************************************************************************************************************/
