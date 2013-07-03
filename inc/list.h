/******************************************************************************
 * This file is part of eSolid-Kernel
 *
 * Copyright (C) 2011, 2012 - Nenad Radulovic
 *
 * eSolid-Kernel is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * eSolid-Kernel is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with eSolid-Kernel; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 *
 * web site:    http://blueskynet.dyndns-server.com
 * e-mail  :    blueskyniss@gmail.com
 *//***********************************************************************//**
 * @file
 * @author      Nenad Radulovic
 * @brief       List implementations
 * @addtogroup  kern_intf
 *********************************************************************//** @{ */

#if !defined(LIST_H_)
#define LIST_H_

/*=========================================================  INCLUDE FILES  ==*/
#include "compiler.h"

/*===============================================================  MACRO's  ==*/

/*
 * Simple doubly linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */

#define LIST_HEAD_INIT(name)            { &(name), &(name) }

#define LIST_HEAD(name) \
	struct esDList name = LIST_HEAD_INIT(name)

/**
 * list_entry - get the struct for this entry
 * @ptr:    the &struct esDList pointer.
 * @type:   the type of the struct this is embedded in.
 * @member: the name of the list_struct within the struct.
 */
#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)

/**
 * list_first_entry - get the first element from a list
 * @ptr:    the list head to take the element from.
 * @type:   the type of the struct this is embedded in.
 * @member: the name of the list_struct within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define list_first_entry(ptr, type, member) \
    list_entry((ptr)->next, type, member)


/**
 * list_for_each    -   iterate over a list
 * @pos:    the &struct esDList to use as a loop cursor.
 * @head:   the head for your list.
 */
#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * __list_for_each  -   iterate over a list
 * @pos:    the &struct esDList to use as a loop cursor.
 * @head:   the head for your list.
 *
 * This variant doesn't differ from list_for_each() any more.
 * We don't do prefetching in either case.
 */
#define __list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * list_for_each_prev   -   iterate over a list backwards
 * @pos:    the &struct esDList to use as a loop cursor.
 * @head:   the head for your list.
 */
#define list_for_each_prev(pos, head) \
    for (pos = (head)->prev; pos != (head); pos = pos->prev)

/**
 * list_for_each_safe - iterate over a list safe against removal of list entry
 * @pos:    the &struct esDList to use as a loop cursor.
 * @n:      another &struct esDList to use as temporary storage
 * @head:   the head for your list.
 */
#define list_for_each_safe(pos, n, head) \
    for (pos = (head)->next, n = pos->next; pos != (head); \
        pos = n, n = pos->next)

/**
 * list_for_each_prev_safe - iterate over a list backwards safe against removal of list entry
 * @pos:    the &struct esDList to use as a loop cursor.
 * @n:      another &struct esDList to use as temporary storage
 * @head:   the head for your list.
 */
#define list_for_each_prev_safe(pos, n, head) \
    for (pos = (head)->prev, n = pos->prev; \
         pos != (head); \
         pos = n, n = pos->prev)

/**
 * list_for_each_entry  -   iterate over list of given type
 * @pos:    the type * to use as a loop cursor.
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 */
#define list_for_each_entry(pos, head, member)              \
    for (pos = list_entry((head)->next, typeof(*pos), member);  \
         &pos->member != (head);    \
         pos = list_entry(pos->member.next, typeof(*pos), member))

/**
 * list_for_each_entry_reverse - iterate backwards over list of given type.
 * @pos:    the type * to use as a loop cursor.
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 */
#define list_for_each_entry_reverse(pos, head, member)          \
    for (pos = list_entry((head)->prev, typeof(*pos), member);  \
         &pos->member != (head);    \
         pos = list_entry(pos->member.prev, typeof(*pos), member))

/**
 * list_prepare_entry - prepare a pos entry for use in list_for_each_entry_continue()
 * @pos:    the type * to use as a start point
 * @head:   the head of the list
 * @member: the name of the list_struct within the struct.
 *
 * Prepares a pos entry for use as a start point in list_for_each_entry_continue().
 */
#define list_prepare_entry(pos, head, member) \
    ((pos) ? : list_entry(head, typeof(*pos), member))

/**
 * list_for_each_entry_continue - continue iteration over list of given type
 * @pos:    the type * to use as a loop cursor.
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 *
 * Continue to iterate over list of given type, continuing after
 * the current position.
 */
#define list_for_each_entry_continue(pos, head, member)         \
    for (pos = list_entry(pos->member.next, typeof(*pos), member);  \
         &pos->member != (head);    \
         pos = list_entry(pos->member.next, typeof(*pos), member))

/**
 * list_for_each_entry_continue_reverse - iterate backwards from the given point
 * @pos:    the type * to use as a loop cursor.
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 *
 * Start to iterate over list of given type backwards, continuing after
 * the current position.
 */
#define list_for_each_entry_continue_reverse(pos, head, member)     \
    for (pos = list_entry(pos->member.prev, typeof(*pos), member);  \
         &pos->member != (head);    \
         pos = list_entry(pos->member.prev, typeof(*pos), member))

/**
 * list_for_each_entry_from - iterate over list of given type from the current point
 * @pos:    the type * to use as a loop cursor.
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 *
 * Iterate over list of given type, continuing from current position.
 */
#define list_for_each_entry_from(pos, head, member)             \
    for (; &pos->member != (head);  \
         pos = list_entry(pos->member.next, typeof(*pos), member))

/**
 * list_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @pos:    the type * to use as a loop cursor.
 * @n:      another type * to use as temporary storage
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 */
#define list_for_each_entry_safe(pos, n, head, member)          \
    for (pos = list_entry((head)->next, typeof(*pos), member),  \
        n = list_entry(pos->member.next, typeof(*pos), member); \
         &pos->member != (head);                    \
         pos = n, n = list_entry(n->member.next, typeof(*n), member))

/**
 * list_for_each_entry_safe_continue - continue list iteration safe against removal
 * @pos:    the type * to use as a loop cursor.
 * @n:      another type * to use as temporary storage
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 *
 * Iterate over list of given type, continuing after current point,
 * safe against removal of list entry.
 */
#define list_for_each_entry_safe_continue(pos, n, head, member)         \
    for (pos = list_entry(pos->member.next, typeof(*pos), member),      \
        n = list_entry(pos->member.next, typeof(*pos), member);     \
         &pos->member != (head);                        \
         pos = n, n = list_entry(n->member.next, typeof(*n), member))

/**
 * list_for_each_entry_safe_from - iterate over list from current point safe against removal
 * @pos:    the type * to use as a loop cursor.
 * @n:      another type * to use as temporary storage
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 *
 * Iterate over list of given type from current point, safe against
 * removal of list entry.
 */
#define list_for_each_entry_safe_from(pos, n, head, member)             \
    for (n = list_entry(pos->member.next, typeof(*pos), member);        \
         &pos->member != (head);                        \
         pos = n, n = list_entry(n->member.next, typeof(*n), member))

/**
 * list_for_each_entry_safe_reverse - iterate backwards over list safe against removal
 * @pos:    the type * to use as a loop cursor.
 * @n:      another type * to use as temporary storage
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 *
 * Iterate backwards over list of given type, safe against removal
 * of list entry.
 */
#define list_for_each_entry_safe_reverse(pos, n, head, member)      \
    for (pos = list_entry((head)->prev, typeof(*pos), member),  \
        n = list_entry(pos->member.prev, typeof(*pos), member); \
         &pos->member != (head);                    \
         pos = n, n = list_entry(n->member.prev, typeof(*n), member))

/**
 * list_safe_reset_next - reset a stale list_for_each_entry_safe loop
 * @pos:    the loop cursor used in the list_for_each_entry_safe loop
 * @n:      temporary storage used in list_for_each_entry_safe
 * @member: the name of the list_struct within the struct.
 *
 * list_safe_reset_next is not safe to use in general if the list may be
 * modified concurrently (eg. the lock is dropped in the loop body). An
 * exception to this is if the cursor element (pos) is pinned in the list,
 * and list_safe_reset_next is called after re-taking the lock and before
 * completing the current iteration of the loop body.
 */
#define list_safe_reset_next(pos, n, member)                \
    n = list_entry(pos->member.next, typeof(*pos), member)

/**@} *//*----------------------------------------------  C++ extern begin  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/

struct esDList {
    struct esDList *   next;
    struct esDList *   prev;
};

typedef struct esDList esDList_T;

/*======================================================  GLOBAL VARIABLES  ==*/
/*===================================================  FUNCTION PROTOTYPES  ==*/

static PORT_C_INLINE void esDListInit_(
    esDList_T *     list) {
	list->next = list;
	list->prev = list;
}

/**@brief       Insert a new entry between two known consecutive entries.
 * @details     This is only for list manipulation where the prev/next entries
 *              are known already.
 */
static PORT_C_INLINE void esDListAdd_(
    esDList_T * newItem,
	esDList_T * prev,
	esDList_T * next) {

	next->prev = newItem;
	newItem->next = next;
	newItem->prev = prev;
	prev->next = newItem;
}

/**@brief       Add a new entry
 * @param       newItem
 *              New entry to be added
 * @param       head
 *              List head to add it after
 * @details     Insert a new entry after the specified head. This is good for
 *              implementing stacks.
 */
static PORT_C_INLINE void esDListAddAfter_(
    esDList_T *     newItem,
    esDList_T *     head) {
	esDListAdd_(
	    newItem,
	    head,
	    head->next);
}

/**@brief       Add a new entry
 * @param       newItem
 *              New entry to be added
 * @param       head
 *              List head to add it before
 * @details     Insert a new entry before the specified head. This is useful for
 *              implementing queues.
 */
static PORT_C_INLINE void esDListAddTail_(
    esDList_T *     newItem,
    esDList_T *     head) {
	esDListAdd_(
	    newItem,
	    head->prev,
	    head);
}

/**@brief       Delete a list entry by making the prev/next entries point to
 *              each other.
 * @details     This is only for list manipulation where the prev/next entries
 *              are known already.
 */
static PORT_C_INLINE void esDListRm_(
    esDList_T *     prev,
    esDList_T *     next) {
	next->prev = prev;
	prev->next = next;
}

/**@brief       Deletes entry from list.
 * @param       entry
 *              The element to delete from the list.
 * @note        esDListIsEmpty_() on entry does not return true after this, the entry is
 *              in an undefined state.
 */
static PORT_C_INLINE void esDListRmEntry_(
    esDList_T *     entry) {
	esDListRm_(
	    entry->prev,
	    entry->next);
}

/**@brief       Replace old entry by new one
 * @param       oldItem
 *              The entry to be replaced
 * @param       newItem
 *              The new entry to insert
 */
static PORT_C_INLINE void esDListReplace_(
    esDList_T *     oldItem,
    esDList_T *     newItem) {
	newItem->next = oldItem->next;
	newItem->next->prev = newItem;
	newItem->prev = oldItem->prev;
	newItem->prev->next = newItem;
}

/**@brief       Delete from one list and add as another's head
 * @param       item
 *              The entry to move
 * @param       head
 *              The head that will precede our entry
 */
static PORT_C_INLINE void esDListMv_(
    esDList_T *     item,
    esDList_T *     head) {
	esDListRmEntry_(
	    item);
	esDListAddAfter_(
	    item,
	    head);
}

/**@brief       Delete from one list and add as another's tail
 * @param       item
 *              The entry to move
 * @param       head
 *              The head that will follow our entry
 */
static PORT_C_INLINE void esDListMvTail_(
    esDList_T *     item,
    esDList_T *     head) {
	esDListRmEntry_(
	    item);
	esDListAddTail_(
	    item,
	    head);
}

/**@brief       Tests whether @item is the last entry in list @head
 * @param       Item
 *              The entry to test
 * @param       head
 *              The head of the list
 */
static PORT_C_INLINE int esDListIsLast_(
    const esDList_T *   item) {

    bool_T state;

    if (item->next == item) {
        state = TRUE;
    } else {
        state = FALSE;
    }

	return (state);
}

/**@brief       Tests whether a list is empty
 * @param       head
 *              The list to test.
 * @return      List state
 *  @retval     TRUE - list is empty
 *  @retval     FALSE - list is not empty
 */
static PORT_C_INLINE bool_T esDListIsEmpty_(
    const esDList_T *   head) {

    bool_T state;

    if (head->next == head) {
        state = TRUE;
    } else {
        state = FALSE;
    }

	return (state);
}

static PORT_C_INLINE esDList_T * esDListGetNext_(
    const esDList_T *   item) {

    return (item->next);
}

/**
 * list_empty_careful - tests whether a list is empty and not being modified
 * @head: the list to test
 *
 * Description:
 * tests whether a list is empty _and_ checks that no other CPU might be
 * in the process of modifying either member (next or prev)
 *
 * NOTE: using list_empty_careful() without synchronization
 * can only be safe if the only activity that can happen
 * to the list entry is list_del_init(). Eg. it cannot be used
 * if another CPU could re-esDListAddAfter_() it.
 */
static PORT_C_INLINE int list_empty_careful(const esDList_T * head)
{
	esDList_T * next = head->next;
	return (next == head) && (next == head->prev);
}

/**
 * list_rotate_left - rotate the list to the left
 * @head: the head of the list
 */
static PORT_C_INLINE void list_rotate_left(esDList_T * head)
{
	esDList_T * first;

	if (!esDListIsEmpty_(head)) {
		first = head->next;
		esDListMvTail_(first, head);
	}
}

/**
 * list_is_singular - tests whether a list has just one entry.
 * @head: the list to test.
 */
static PORT_C_INLINE int list_is_singular(const esDList_T * head)
{
	return !esDListIsEmpty_(head) && (head->next == head->prev);
}

static PORT_C_INLINE void __list_cut_position(esDList_T * list,
		esDList_T * head, esDList_T * entry)
{
	esDList_T * new_first = entry->next;
	list->next = head->next;
	list->next->prev = list;
	list->prev = entry;
	entry->next = list;
	head->next = new_first;
	new_first->prev = head;
}

/**
 * list_cut_position - cut a list into two
 * @list: a new list to add all removed entries
 * @head: a list with entries
 * @entry: an entry within head, could be the head itself
 *	and if so we won't cut the list
 *
 * This helper moves the initial part of @head, up to and
 * including @entry, from @head to @list. You should
 * pass on @entry an element you know is on @head. @list
 * should be an empty list or a list you do not care about
 * losing its data.
 *
 */
static PORT_C_INLINE void list_cut_position(esDList_T * list,
		esDList_T * head, esDList_T * entry)
{
	if (esDListIsEmpty_(head))
		return;
	if (list_is_singular(head) &&
		(head->next != entry && head != entry))
		return;
	if (entry == head)
		esDListInit_(list);
	else
		__list_cut_position(list, head, entry);
}

static PORT_C_INLINE void __list_splice(const esDList_T * list,
				 esDList_T * prev,
				 esDList_T * next)
{
	esDList_T * first = list->next;
	esDList_T * last = list->prev;

	first->prev = prev;
	prev->next = first;

	last->next = next;
	next->prev = last;
}

/**
 * list_splice - join two lists, this is designed for stacks
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
static PORT_C_INLINE void list_splice(const esDList_T * list,
				esDList_T * head)
{
	if (!esDListIsEmpty_(list))
		__list_splice(list, head, head->next);
}

/**
 * list_splice_tail - join two lists, each list being a queue
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
static PORT_C_INLINE void list_splice_tail(esDList_T * list,
				esDList_T * head)
{
	if (!esDListIsEmpty_(list))
		__list_splice(list, head->prev, head);
}

/**
 * list_splice_init - join two lists and reinitialise the emptied list.
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 *
 * The list at @list is reinitialised
 */
static PORT_C_INLINE void list_splice_init(esDList_T * list,
				    esDList_T * head)
{
	if (!esDListIsEmpty_(list)) {
		__list_splice(list, head, head->next);
		esDListInit_(list);
	}
}

/**
 * list_splice_tail_init - join two lists and reinitialise the emptied list
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 *
 * Each of the lists is a queue.
 * The list at @list is reinitialised
 */
static PORT_C_INLINE void list_splice_tail_init(esDList_T * list,
					 esDList_T * head)
{
	if (!esDListIsEmpty_(list)) {
		__list_splice(list, head->prev, head);
		esDListInit_(list);
	}
}

/**@} *//*------------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//**@} *//*******************************************************
 * END of list.h
 ******************************************************************************/
#endif
