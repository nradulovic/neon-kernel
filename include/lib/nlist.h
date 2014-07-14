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
 * @brief       Linked lists header
 * @defgroup    linked_list Linked lists
 * @brief       Linked lists
 *********************************************************************//** @{ */

#ifndef NLIST_H
#define NLIST_H

/*=========================================================  INCLUDE FILES  ==*/

#include <stdbool.h>

#include "plat/compiler.h"

/*===============================================================  MACRO's  ==*/
/*------------------------------------------------------  C++ extern begin  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/

struct ndlist
{
    struct ndlist *     next;
    struct ndlist *     prev;
};

/*======================================================  GLOBAL VARIABLES  ==*/
/*===================================================  FUNCTION PROTOTYPES  ==*/

static PORT_C_INLINE void ndlist_init(
    struct ndlist *             node)
{
    node->next = node;
    node->prev = node;
}

static PORT_C_INLINE void ndlist_add_before(
    struct ndlist *             current,
    struct ndlist *             node)
{
    node->next          = current;
    node->prev          = current->prev;
    current->prev->next = node;
    current->prev       = node;
}

static PORT_C_INLINE void ndlist_add_after(
    struct ndlist *             current,
    struct ndlist *             node)
{
    node->prev          = current;
    node->next          = current->next;
    current->next->prev = node;
    current->next       = node;
}

static PORT_C_INLINE void ndlist_remove(
    struct ndlist *             node)
{
    node->next->prev = node->prev;
    node->prev->next = node->next;
}

static PORT_C_INLINE bool ndlist_is_empty(
    const struct ndlist *       node)
{
    if (node->next == node) {
        return (true);
    } else {
        return (false);
    }
}

static PORT_C_INLINE struct ndlist * ndlist_next(
    const struct ndlist *       node)
{
    return (node->next);
}

static PORT_C_INLINE struct ndlist * ndlist_prev(
    const struct ndlist *       node)
{
    return (node->prev);
}

/*--------------------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of nlist.h
 ******************************************************************************/
#endif /* NLIST_H */
