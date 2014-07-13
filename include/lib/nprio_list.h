/*
 * This file is part of NUB Real-Time Kernel.
 *
 * Copyright (C) 2010 - 2014 Nenad Radulovic
 *
 * NUB Real-Time Kernel is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * NUB Real-Time Kernel is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with NUB Real-Time Kernel.  If not, see <http://www.gnu.org/licenses/>.
 *
 * web site:    http://github.com/nradulovic
 * e-mail  :    nenad.b.radulovic@gmail.com
 *//***********************************************************************//**
 * @file
 * @author  	Nenad Radulovic
 * @brief       Priority linked lists header
 * @defgroup    prio_linked_list Priority linked lists
 * @brief       Priority linked lists
 *********************************************************************//** @{ */

#ifndef NPRIO_LIST_H
#define NPRIO_LIST_H

/*=========================================================  INCLUDE FILES  ==*/

#include <stdbool.h>
#include <stdint.h>

#include "plat/compiler.h"
#include "lib/nlist.h"

/*===============================================================  MACRO's  ==*/

#define NDLIST_TO_PRIO_DLIST(node)                                              \
    container_of(node, struct nprio_list_node, list)

/*------------------------------------------------------  C++ extern begin  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/

struct nprio_list
{
    struct ndlist               list;
};

struct nprio_list_node
{
    struct ndlist               list;
    uint_fast8_t                priority;
};

/*======================================================  GLOBAL VARIABLES  ==*/
/*===================================================  FUNCTION PROTOTYPES  ==*/

static PORT_C_INLINE void nprio_list_init_node(
    struct nprio_list_node *    node,
    uint_fast8_t                priority)
{
    ndlist_init(&node->list);
    node->priority = priority;
}



static PORT_C_INLINE void nprio_list_init(
    struct nprio_list *         list)
{
    ndlist_init(&list->list);
}



static PORT_C_INLINE void nprio_list_prio_insert(
    struct nprio_list *         list,
    struct nprio_list_node *    node)
{
    struct ndlist *             current;

    current = &list->list;

    do {
        current = ndlist_next(current);                                         /* Iterate to the next node in list.  */
    } while ((current != &list->list) &&                                        /* Not end of list and node has equal */
             (NDLIST_TO_PRIO_DLIST(current)->priority >= node->priority));      /* or higher priority than given node?*/
    ndlist_add_before(current, &node->list);
}



static PORT_C_INLINE void nprio_list_fifo_insert(
    struct nprio_list *         list,
    struct nprio_list_node *    node)
{
    ndlist_add_before(&list->list, &node->list);
}



static PORT_C_INLINE void nprio_list_remove(
    struct nprio_list_node *    node)
{
    ndlist_remove(&node->list);
}



static PORT_C_INLINE struct nprio_list_node * nprio_list_tail(
    const struct nprio_list *   list)
{
    return (NDLIST_TO_PRIO_DLIST(ndlist_next(&list->list)));
}



static PORT_C_INLINE struct nprio_list_node * nprio_list_head(
    struct nprio_list *        list)
{
    return (NDLIST_TO_PRIO_DLIST(ndlist_prev(&list->list)));
}



static PORT_C_INLINE bool nprio_list_is_empty(
    const struct nprio_list *  list)
{
    if (ndlist_is_empty(&list->list)) {
        return (true);
    } else {
        return (false);
    }
}



static PORT_C_INLINE struct nprio_list_node * nprio_list_next(
    const struct nprio_list_node * node)
{
    return (NDLIST_TO_PRIO_DLIST(ndlist_next(&node->list)));
}



static PORT_C_INLINE uint_fast8_t nprio_list_priority(
    const struct nprio_list_node * node)
{
    return (node->priority);
}



static PORT_C_INLINE void nprio_list_set_priority(
    struct nprio_list_node *    node,
    uint_fast8_t                priority)
{
    node->priority = priority;
}

/*--------------------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of nprio_list.h
 ******************************************************************************/
#endif /* NPRIO_LIST_H */
