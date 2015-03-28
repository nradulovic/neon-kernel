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
 * @author      Nenad Radulovic
 * @brief       Priority queue header
 * @defgroup    priority_queue Priority queue
 * @brief       Priority queue
 *********************************************************************//** @{ */
/**@defgroup    priority_queue_intf Interface
 * @brief       Public interface
 * @{ *//*--------------------------------------------------------------------*/

#ifndef NPRIO_QUEUE_H_
#define NPRIO_QUEUE_H_

/*=========================================================  INCLUDE FILES  ==*/

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "base/port/compiler.h"
#include "base/port/core.h"
#include "kernel/lib/bitop.h"

/*===============================================================  MACRO's  ==*/
/*------------------------------------------------------  C++ extern begin  --*/
#ifdef __cplusplus
extern "C" {
#endif

#define NBITMAP_DIM(max_index)                                                  \
    NDIVISION_ROUNDUP((max_index), NCPU_DATA_WIDTH)

#define NBITMAP_IS_SINGLE(max_index)                                            \
    ((max_index) <= NCPU_DATA_WIDTH)

/*============================================================  DATA TYPES  ==*/

struct nbitmap
{
    ncpu_reg                    group;
};

/*======================================================  GLOBAL VARIABLES  ==*/
/*===================================================  FUNCTION PROTOTYPES  ==*/


PORT_C_INLINE
void nbitmap_multi_init(
    struct nbitmap *            bitmap,
    size_t                      size)
{
    size /= sizeof(struct nbitmap [1]);

    while (size-- != 0) {
        bitmap[size].group = 0u;
    }
}



PORT_C_INLINE
void nbitmap_multi_set(
    struct nbitmap *            bitmap,
    uint_fast8_t                index)
{
    uint_fast8_t                group_index;
    uint_fast8_t                bit_index;

    bit_index   = index & ((uint_fast8_t)~0u >> (sizeof(index) * 8u -
        NLOG2_8(NCPU_DATA_WIDTH)));
    group_index = index >> NLOG2_8(NCPU_DATA_WIDTH);
    bitmap[0].group                |= ncpu_exp2(group_index);
    bitmap[group_index + 1u].group |= ncpu_exp2(bit_index);
}



PORT_C_INLINE
void nbitmap_multi_clear(
    struct nbitmap *            bitmap,
    uint_fast8_t                index)
{
    uint_fast8_t                group_index;
    uint_fast8_t                bit_index;

    bit_index   = index & ((uint_fast8_t)~0u >> (sizeof(index) * 8u -
            NLOG2_8(NCPU_DATA_WIDTH)));
    group_index = index >> NLOG2_8(NCPU_DATA_WIDTH);
    bitmap[group_index + 1u].group &= ~ncpu_exp2(bit_index);

    if (bitmap[group_index + 1u].group == 0u) {
                                        /* If this is the last bit cleared in */
                                        /* this row then clear bit in group   */
                                        /* indicator, too.                    */
        bitmap[0].group &= ~ncpu_exp2(group_index);
    }
}



PORT_C_INLINE
uint_fast8_t nbitmap_multi_get_highest(
    const struct nbitmap *      bitmap)
{
    uint_fast8_t                group_index;
    uint_fast8_t                bit_index;

    group_index = ncpu_log2(bitmap[0].group);
    bit_index   = ncpu_log2(bitmap[group_index + 1u].group);

    return ((uint_fast8_t)(group_index << NLOG2_8(NCPU_DATA_WIDTH)) | bit_index);
}



PORT_C_INLINE
void nbitmap_single_init(
    struct nbitmap *            bitmap)
{
    bitmap[0].group = 0u;
}



PORT_C_INLINE
void nbitmap_single_set(
    struct nbitmap *            bitmap,
    uint_fast8_t                index)
{
    bitmap[0].group |= ncpu_exp2(index);
}



PORT_C_INLINE
void nbitmap_single_clear(
    struct nbitmap *            bitmap,
    uint_fast8_t                index)
{
    bitmap[0].group &= ~ncpu_exp2(index);
}



PORT_C_INLINE
uint_fast8_t nbitmap_single_get_highest(
    const struct nbitmap *      bitmap)
{
    uint_fast8_t                index;

    index = ncpu_log2(bitmap[0].group);

    return (index);
}



PORT_C_INLINE
bool nbitmap_is_empty(
    const struct nbitmap *      bitmap)
{
    if (bitmap[0].group == 0u) {
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
/** @endcond *//** @} *//** @} *//*********************************************
 * END of prio_queue.h
 ******************************************************************************/
#endif /* NPRIO_QUEUE_H_ */
