/*
 * This file is part of Neon.
 *
 * Copyright (C) 2010 - 2015 Nenad Radulovic
 *
 * Neon is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Neon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Neon.  If not, see <http://www.gnu.org/licenses/>.
 *
 * web site:    http://github.com/nradulovic
 * e-mail  :    nenad.b.radulovic@gmail.com
 *//***********************************************************************//**
 * @file
 * @author      Nenad Radulovic
 * @brief       Scheduler implementation
 * @addtogroup  sched
 *********************************************************************//** @{ */
/**@defgroup    sched_impl Implementation
 * @brief       Scheduler Implementation
 * @{ *//*--------------------------------------------------------------------*/

/*=========================================================  INCLUDE FILES  ==*/

#include <string.h>

#include "port/sys_lock.h"
#include "port/cpu.h"
#include "shared/component.h"
#include "lib/bitop.h"
#include "lib/bitmap.h"
#include "kernel/sched.h"

/*=========================================================  LOCAL MACRO's  ==*/

#define NODE_TO_THREAD(node_ptr)                                                \
    CONTAINER_OF(node_ptr, struct nthread, node)

#define NPRIO_ARRAY_BUCKET_BITS                                                 \
    NLOG2_8(NDIVISION_ROUNDUP(CONFIG_PRIORITY_LEVELS, CONFIG_PRIORITY_BUCKETS))

#if (NBITMAP_IS_SINGLE(CONFIG_PRIORITY_BUCKETS))
#define BITMAP_INIT(bitmap)                 nbitmap_single_init((bitmap))
#define BITMAP_SET(bitmap, index)           nbitmap_single_set((bitmap), (index))
#define BITMAP_CLEAR(bitmap, index)         nbitmap_single_clear((bitmap), (index))
#define BITMAP_GET_HIGHEST(bitmap)          nbitmap_single_get_highest((bitmap))
#define BITMAP_IS_EMPTY(bitmap)             nbitmap_is_empty((bitmap))
#else
#define BITMAP_INIT(bitmap)                 nbitmap_multi_init((bitmap), sizeof((bitmap)))
#define BITMAP_SET(bitmap, index)           nbitmap_multi_set((bitmap), (index))
#define BITMAP_CLEAR(bitmap, index)         nbitmap_multi_clear((bitmap), (index))
#define BITMAP_GET_HIGHEST(bitmap)          nbitmap_multi_get_highest((bitmap))
#define BITMAP_IS_EMPTY(bitmap)             nbitmap_is_empty((bitmap))
#endif

/*======================================================  LOCAL DATA TYPES  ==*/

/**@brief       Priority queue structure
 * @details     A priority queue consists of an array of sub-queues. There is
 *              one sub-queue per priority level. Each sub-queue contains the
 *              nodes at the corresponding priority level. There is also a
 *              bitmap corresponding to the array that is used to determine
 *              effectively the highest priority node on the queue.
 * @api
 */
struct prio_queue
{
#if (CONFIG_PRIORITY_BUCKETS != 1)
    struct nbitmap              bitmap[NBITMAP_DIM(CONFIG_PRIORITY_BUCKETS)];
                                        /**<@brief Priority bitmap            */
#endif  /* (CONFIG_PRIORITY_BUCKETS != 1) */
    struct nbias_list *         sentinel[CONFIG_PRIORITY_BUCKETS];
};

/**@brief       Scheduler context structure
 * @details     This structure holds important status data for the scheduler.
 */
struct sched_ctx
{
    struct nbias_list *         current;    /**<@brief The current thread     */
    struct prio_queue          run_queue;  /**<@brief Run queue of threads   */
    void                     (* idle)(void);
};

/*=============================================  LOCAL FUNCTION PROTOTYPES  ==*/

PORT_C_INLINE
void prio_queue_init(
    struct prio_queue *         queue);



PORT_C_INLINE
void prio_queue_insert(
    struct prio_queue *         queue,
    struct nbias_list  *        node);



PORT_C_INLINE
void prio_queue_remove(
    struct prio_queue *         queue,
    struct nbias_list  *        node);



PORT_C_INLINE
void prio_queue_rotate(
    struct prio_queue *         queue,
    struct nbias_list  *        node);



PORT_C_INLINE
struct nbias_list * prio_queue_peek(
    const struct prio_queue *   queue);



PORT_C_INLINE
bool prio_queue_is_empty(
    const struct prio_queue *   queue);

/*=======================================================  LOCAL VARIABLES  ==*/

static struct sched_ctx         g_sched_ctx;

/*======================================================  GLOBAL VARIABLES  ==*/
/*============================================  LOCAL FUNCTION DEFINITIONS  ==*/


PORT_C_INLINE
void prio_queue_init(
    struct prio_queue *         queue)
{
    uint_fast8_t                count;

#if (CONFIG_PRIORITY_BUCKETS != 1)
    BITMAP_INIT(queue->bitmap);
#endif
    count = NARRAY_DIMENSION(queue->sentinel);

    while (count-- != 0u) {                                                     /* Initialize each list entry.        */
        queue->sentinel[count] = NULL;
    }
}



PORT_C_INLINE
void prio_queue_insert(
    struct prio_queue *         queue,
    struct nbias_list  *        node)
{
    uint_fast8_t                bucket;

#if (CONFIG_PRIORITY_BUCKETS != 1)
    bucket = (uint_fast8_t)nbias_list_get_bias(node) >> NPRIO_ARRAY_BUCKET_BITS;
#else
    bucket = 0u;
#endif
                                        /* If adding the first entry.         */
    if (queue->sentinel[bucket] == NULL) {
        queue->sentinel[bucket] = node;
#if (CONFIG_PRIORITY_BUCKETS != 1)
                                        /* Mark the bucket list as used.      */
        BITMAP_SET(queue->bitmap, bucket);
#endif
    } else {
#if (CONFIG_PRIORITY_BUCKETS != CONFIG_PRIORITY_LEVELS)
                                        /* Priority search and insertion.     */
        nbias_list_sort_insert(queue->sentinel[bucket], node);
#else
                                        /* FIFO insertion.                    */
        nbias_list_fifo_insert(queue->sentinel[bucket], node);
#endif
    }
}



PORT_C_INLINE
void prio_queue_remove(
    struct prio_queue *         queue,
    struct nbias_list  *        node)
{
    uint_fast8_t                bucket;

#if (CONFIG_PRIORITY_BUCKETS != 1)
    bucket = (uint_fast8_t)nbias_list_get_bias(node) >> NPRIO_ARRAY_BUCKET_BITS;
#else
    bucket = 0u;
#endif

    if (nbias_list_is_empty(node)) {    /* If this was the last node in list. */
        queue->sentinel[bucket] = NULL;
#if (CONFIG_PRIORITY_BUCKETS != 1)
        BITMAP_CLEAR(queue->bitmap, bucket);
                                        /* Mark the bucket as unused.         */
#endif
    } else {
        nbias_list_remove(node);
    }
}



PORT_C_INLINE
void prio_queue_rotate(
    struct prio_queue *         queue,
    struct nbias_list  *        node)
{
    uint_fast8_t                bucket;

#if (CONFIG_PRIORITY_BUCKETS != 1)
    bucket = (uint_fast8_t)nbias_list_get_bias(node) >> NPRIO_ARRAY_BUCKET_BITS;
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



PORT_C_INLINE
struct nbias_list * prio_queue_peek(
    const struct prio_queue *   queue)
{
    uint_fast8_t                bucket;

#if (CONFIG_PRIORITY_BUCKETS != 1)
    bucket = BITMAP_GET_HIGHEST(queue->bitmap);
#else
    bucket = 0u;
#endif

    return (nbias_list_tail(queue->sentinel[bucket]));
}



PORT_C_INLINE
bool prio_queue_is_empty(
    const struct prio_queue *   queue)
{
#if (CONFIG_PRIORITY_BUCKETS != 1)
    return (BITMAP_IS_EMPTY(queue->bitmap));
#else
    if (queue->sentinel[0] == NULL) {
        return (true);
    } else {
        return (false);
    }
#endif
}
/*===================================  GLOBAL PRIVATE FUNCTION DEFINITIONS  ==*/
/*====================================  GLOBAL PUBLIC FUNCTION DEFINITIONS  ==*/

void nsched_init(void)
{
    struct sched_ctx *          ctx = &g_sched_ctx;

    ctx->current = NULL;
    prio_queue_init(&ctx->run_queue);     /* Initialize run_queue structure. */
}



void nsched_term(void)
{
    struct sched_ctx *          ctx = &g_sched_ctx;

    ctx->current = NULL;
}



void nsched_thread_init(
    struct nthread *            thread,
    const struct nthread_define * define)
{
    nbias_list_init(&thread->node, define->priority);
    thread->ref = 0;

#if (CONFIG_REGISTRY == 1)
    strncpy(thread->name, define->name, sizeof(thread->name));
    ndlist_init(&thread->registry_node);
#endif
}



void nsched_thread_term(struct nthread * thread)
{
    struct sched_ctx *          ctx = &g_sched_ctx;
    nsys_lock                   sys_lock;

    nsys_lock_enter(&sys_lock);

    if (thread->ref != 0u) {
        thread->ref =  0u;
        prio_queue_remove(&ctx->run_queue, &thread->node);
    }
    nbias_list_term(&thread->node);
    nsys_lock_exit(&sys_lock);
}



void nsched_thread_insert_i(struct nthread * thread)
{
    ncpu_sat_increment(&thread->ref);

    if (thread->ref == 1u) {
        struct sched_ctx *      ctx = &g_sched_ctx;

        prio_queue_insert(&ctx->run_queue, &thread->node);
    }
}



void nsched_thread_remove_i(struct nthread * thread)
{
    if (thread->ref == 1u) {
        struct sched_ctx *      ctx = &g_sched_ctx;

        prio_queue_remove(&ctx->run_queue, &thread->node);
    }
    ncpu_sat_decrement(&thread->ref);
}



struct nthread * nsched_thread_fetch_i(void)
{
    struct sched_ctx *          ctx = &g_sched_ctx;
    struct nbias_list *         new_node;

    if (!prio_queue_is_empty(&ctx->run_queue)) {
        new_node = prio_queue_peek(&ctx->run_queue);
        prio_queue_rotate(&ctx->run_queue, new_node);
        ctx->current = new_node;

        return (NODE_TO_THREAD(new_node));
    } else {
        ctx->current = NULL;

        return (NULL);
    }
}



struct nthread * nsched_get_current(void)
{
    struct sched_ctx *          ctx = &g_sched_ctx;

    return (NODE_TO_THREAD(ctx->current));
}

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//** @} *//*********************************************
 * END of equeue.c
 ******************************************************************************/
