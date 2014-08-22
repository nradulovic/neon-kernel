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
 * @brief       Bias linked lists header
 * @defgroup    bias_linked_list Bias linked lists
 * @brief       Bias linked lists
 *********************************************************************//** @{ */

#ifndef NBIAS_LIST_H
#define NBIAS_LIST_H

/*=========================================================  INCLUDE FILES  ==*/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "plat/compiler.h"
#include "lib/nlist.h"

/*===============================================================  MACRO's  ==*/

#define NDLIST_TO_BIAS_LIST(node)                                               \
    CONTAINER_OF(node, struct nbias_list, list)

/*------------------------------------------------------  C++ extern begin  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/

struct nbias_list
{
    struct ndlist               list;
    uint_fast32_t               bias;
};

/*======================================================  GLOBAL VARIABLES  ==*/
/*===================================================  FUNCTION PROTOTYPES  ==*/


PORT_C_INLINE
void nbias_list_init(
    struct nbias_list *         node,
    uint_fast32_t               bias)
{
    ndlist_init(&node->list);
    node->bias = bias;
}



PORT_C_INLINE
void nbias_list_sort_insert(
    struct nbias_list *         list,
    struct nbias_list *         node)
{
    struct ndlist *             current;

    current = &list->list;

    do {
        current = ndlist_next(current); /* Iterate to the next node in list.  */
                                        /* Not end of list and node has equal */
                                        /* or higher bias than given node?    */
    } while ((current != &list->list) &&                                        
             (NDLIST_TO_BIAS_LIST(current)->bias >= node->bias));               
    ndlist_add_before(current, &node->list);
}



PORT_C_INLINE
void nbias_list_fifo_insert(
    struct nbias_list *         list,
    struct nbias_list *         node)
{
    ndlist_add_before(&list->list, &node->list);
}



PORT_C_INLINE
void nbias_list_remove(
    struct nbias_list *         node)
{
    ndlist_remove(&node->list);
}



PORT_C_INLINE
struct nbias_list * nbias_list_tail(
    const struct nbias_list *   list)
{
    return (NDLIST_TO_BIAS_LIST(ndlist_next(&list->list)));
}



PORT_C_INLINE
struct nbias_list * nbias_list_head(
    struct nbias_list *        list)
{
    return (NDLIST_TO_BIAS_LIST(ndlist_prev(&list->list)));
}



PORT_C_INLINE
bool nbias_list_is_empty(
    const struct nbias_list *  list)
{
    if (ndlist_is_empty(&list->list)) {
        return (true);
    } else {
        return (false);
    }
}



PORT_C_INLINE
struct nbias_list * nbias_list_next(
    const struct nbias_list *   node)
{
    return (NDLIST_TO_BIAS_LIST(ndlist_next(&node->list)));
}



PORT_C_INLINE
uint_fast32_t nbias_list_get_bias(
    const struct nbias_list *   node)
{
    return (node->bias);
}



PORT_C_INLINE
void nbias_list_set_bias(
    struct nbias_list *         node,
    uint_fast32_t               bias)
{
    node->bias = bias;
}

/*--------------------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of nbias_list.h
 ******************************************************************************/
#endif /* NBIAS_LIST_H */
