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
 * @author  	Nenad Radulovic
 * @brief       Linked lists header
 * @defgroup    linked_list Linked lists
 * @brief       Linked lists
 *************************************************************************************************************//** @{ */

#ifndef NLIST_H_
#define NLIST_H_

/*=================================================================================================  INCLUDE FILES  ==*/
/*=======================================================================================================  MACRO's  ==*/

/*-----------------------------------------------------------------------------------------------------------------*//**
 * @name        Singly linked circular list with Sentinel
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

#define NSLIST_INIT(list, node)                                                                                         \
    (node)->list.next = (node)

#define NSLIST_NEXT(list, current)                                                                                      \
    (current)->list.next

#define NSLIST_ADD_AFTER(list, current, node)                                                                           \
    do                                                                                                                  \
    {                                                                                                                   \
        (node)->list.next    = (current)->list.next;                                                                    \
        (current)->list.next = (node);                                                                                  \
    }                                                                                                                   \
    while (0)

#define NSLIST_ADD_HEAD(list, sentinel, node)                                                                           \
    NSLIST_ADD_AFTER(list, sentinel, node)

#define NSLIST_IS_EMPTY(list, sentinel)                                                                                 \
    ((sentinel)->list.next == (sentinel))

#define NSLIST_REMOVE_NEXT(list, current)                                                                               \
    (current)->list.next = (current)->list.next->list.next

#define NSLIST_EACH(list, sentinel, pointer)                                                                            \
    (pointer) = (sentinel)->list.next; (pointer) != (sentinel); (pointer) = (pointer)->list.next

#define NSLIST_EACH_FROM(list, sentinel, pointer, from)                                                                 \
    (pointer) = (from); (pointer) != (sentinel); (pointer) = (pointer)->list.next

#define NSLIST_EACH_TO(list, sentinel, pointer, to)                                                                     \
    (pointer) = (sentinel)->list.next; (pointer) != (to); (pointer) = (pointer)->list.next

/** @} *//*-------------------------------------------------------------------------------------------------------*//**
 * @name        Doubly Linked circular list with Sentinel
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

/**@brief       Initialize node
 */
#define NDLIST_INIT(list, node)                                                                                         \
    do                                                                                                                  \
    {                                                                                                                   \
        (node)->list.next = (node);                                                                                     \
        (node)->list.prev = (node);                                                                                     \
    }                                                                                                                   \
    while (0)

#define NDLIST_IS_EMPTY(list, sentinel)                                                                                 \
    ((sentinel)->list.next == (sentinel))

/**@brief       Get the next node
 */
#define NDLIST_NEXT(list, node)                                                                                         \
    (node)->list.next

/**@brief       Get the previous node
 */
#define NDLIST_PREV(list, node)                                                                                         \
    (node)->list.prev

/**@brief       Add new @c node before @c current node
 */
#define NDLIST_ADD_BEFORE(list, current, node)                                                                          \
    do                                                                                                                  \
    {                                                                                                                   \
        (node)->list.next = (current);                                                                                  \
        (node)->list.prev = (current)->list.prev;                                                                       \
        (current)->list.prev->list.next = (node);                                                                       \
        (current)->list.prev = (node);                                                                                  \
    }                                                                                                                   \
    while (0)

/**@brief       Add new @c node after @c current node
 */
#define NDLIST_ADD_AFTER(list, current, node)                                                                           \
    do                                                                                                                  \
    {                                                                                                                   \
        (node)->list.prev = (current);                                                                                  \
        (node)->list.next = (current)->list.next;                                                                       \
        (current)->list.next->list.prev = (node);                                                                       \
        (current)->list.next = (node);                                                                                  \
    }                                                                                                                   \
    while (0)

#define NDLIST_ADD_HEAD(list, sentinel, node)                                                                           \
    NDLIST_ADD_AFTER(list, sentinel, node)

#define NDLIST_ADD_TAIL(list, sentinel, node)                                                                           \
    NDLIST_ADD_BEFORE(list, sentinel, node)

/**@brief       Remove the @c node from a list
 */
#define NDLIST_REMOVE(list, node)                                                                                       \
    do                                                                                                                  \
    {                                                                                                                   \
        (node)->list.next->list.prev = (node)->list.prev;                                                               \
        (node)->list.prev->list.next = (node)->list.next;                                                               \
    }                                                                                                                   \
    while (0u)

#define NDLIST_EACH(list, sentinel, pointer)                                                                            \
    (pointer) = (sentinel)->list.next; (pointer) != (sentinel); (pointer) = (pointer)->list.next

#define NDLIST_EACH_FROM(list, sentinel, pointer, from)                                                                 \
    (pointer) = (from); (pointer) != (sentinel); (pointer) = (pointer)->list.next

#define NDLIST_EACH_TO(list, sentinel, pointer, to)                                                                     \
    (pointer) = (sentinel)->list.next; (pointer) != (to); (pointer) = (pointer)->list.next

/** @} *//*-------------------------------------------------------------------------------------  C++ extern begin  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*====================================================================================================  DATA TYPES  ==*/
/*==============================================================================================  GLOBAL VARIABLES  ==*/
/*===========================================================================================  FUNCTION PROTOTYPES  ==*/
/** @} *//*---------------------------------------------------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*========================================================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//**********************************************************************************************
 * END of nlist.h
 **********************************************************************************************************************/
#endif /* NLIST_H_ */
