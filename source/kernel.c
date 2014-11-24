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
 * @brief       Thread implementation
 * @addtogroup  thread
 *********************************************************************//** @{ */
/**@defgroup    thread_impl Implementation
 * @brief       Implementation
 * @{ *//*--------------------------------------------------------------------*/

/*=========================================================  INCLUDE FILES  ==*/



#include "kernel/kernel_config.h"

#include "port/compiler.h"
#include "port/sys_lock.h"
#include "port/cpu.h"
#include "shared/component.h"
#include "shared/debug.h"
#include "lib/list.h"
#include "lib/bias_list.h"
#include "lib/bitmap.h"
#include "kernel/kernel.h"

/*=========================================================  LOCAL MACRO's  ==*/

/**@brief       Thread structure signature.
 * @details     The signature is used to confirm that a structure passed to a
 *              function is indeed a nthread thread structure.
 */
#define THREAD_SIGNATURE                    ((ncpu_reg)0xfeedbeeful)

#define NODE_TO_THREAD(node)                                                    \
    CONTAINER_OF(node, struct nthread, queue_node)

#define NPRIO_ARRAY_BUCKET_BITS                                                 \
    NLOG2_8(NDIVISION_ROUNDUP(CONFIG_PRIORITY_LEVELS, CONFIG_PRIORITY_BUCKETS))

#if (NBITMAP_IS_SINGLE(CONFIG_PRIORITY_BUCKETS))
#define BITMAP_INIT(bitmap)					nbitmap_single_init((bitmap))
#define BITMAP_SET(bitmap, index)			nbitmap_single_set((bitmap), (index))
#define BITMAP_CLEAR(bitmap, index)			nbitmap_single_clear((bitmap), (index))
#define BITMAP_GET_HIGHEST(bitmap)			nbitmap_single_get_highest((bitmap))
#define BITMAP_IS_EMPTY(bitmap)				nbitmap_is_empty((bitmap))
#else
#define BITMAP_INIT(bitmap)					nbitmap_multi_init((bitmap), sizeof((bitmap)))
#define BITMAP_SET(bitmap, index)			nbitmap_multi_set((bitmap), (index))
#define BITMAP_CLEAR(bitmap, index)			nbitmap_multi_clear((bitmap), (index))
#define BITMAP_GET_HIGHEST(bitmap)			nbitmap_multi_get_highest((bitmap))
#define BITMAP_IS_EMPTY(bitmap)				nbitmap_is_empty((bitmap))
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
struct nprio_queue
{
#if (CONFIG_PRIORITY_BUCKETS != 1)
    struct nbitmap 				bitmap[NBITMAP_DIM(CONFIG_PRIORITY_BUCKETS)];
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
    struct nprio_queue          run_queue;  /**<@brief Run queue of threads   */
};

/**@brief       System domain structure
 * @details     This structure holds all local data for the whole system. If the
 *              kernel is used in preemptable environment then this structure
 *              will be allocated one per thread. If the underlying OS has fast
 *              synchronisation mechanism then all critical sections can be
 *              protected by using the said mechanism.
 */
struct sys_domain
{
    struct sched_ctx            sched;
};

/*=============================================  LOCAL FUNCTION PROTOTYPES  ==*/


PORT_C_INLINE
void nprio_queue_init(
    struct nprio_queue *        queue);



PORT_C_INLINE
void nprio_queue_insert(
    struct nprio_queue *        queue,
    struct nbias_list  *        node);



PORT_C_INLINE
void nprio_queue_remove(
    struct nprio_queue *        queue,
    struct nbias_list  *        node);



PORT_C_INLINE
void nprio_queue_rotate(
    struct nprio_queue *        queue,
    struct nbias_list  *        node);



PORT_C_INLINE
struct nbias_list * nprio_queue_peek(
    const struct nprio_queue *  queue);



PORT_C_INLINE
bool nprio_queue_is_empty(
    const struct nprio_queue *  queue);



static void sched_init(
    struct sched_ctx *          ctx);



static struct nbias_list * sched_get_current(
    const struct sched_ctx *    ctx);



static void sched_insert_i(
    struct sched_ctx *          ctx,
    struct nbias_list *         thread_node);



static void sched_remove_i(
    struct sched_ctx *          ctx,
    struct nbias_list *         thread_node);



static struct nbias_list * sched_fetch_i(
    struct sched_ctx *          ctx);



#if   (CONFIG_PREEMPT == 1)
static struct nbias_list * sched_fetch_masked_i(
    struct sched_ctx *          ctx,
    uint_fast8_t                mask);
#endif



static void sched_run_i(
    struct sched_ctx *          ctx,
    struct nsys_lock *          lock);



#if   (CONFIG_PREEMPT == 1)
static void sched_preempt_i(
    struct sched_ctx *          ctx,
    struct nsys_lock *          lock);
#endif

/*=======================================================  LOCAL VARIABLES  ==*/

/**@brief       Provides the basic information about this module
 */
static const NCOMPONENT_DEFINE("Neon RT Kernel", "Nenad Radulovic");

static struct sys_domain        g_domain;

/*======================================================  GLOBAL VARIABLES  ==*/
/*============================================  LOCAL FUNCTION DEFINITIONS  ==*/


PORT_C_INLINE
void nprio_queue_init(
    struct nprio_queue *        queue)
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
void nprio_queue_insert(
    struct nprio_queue *        queue,
    struct nbias_list  *        node)
{
    uint_fast8_t                bucket;

#if (CONFIG_PRIORITY_BUCKETS != 1)
    bucket = nbias_list_get_bias(node) >> NPRIO_ARRAY_BUCKET_BITS;
#else
    bucket = 0u;
#endif

    if (queue->sentinel[bucket] == NULL) {
    									/* If adding the first entry.         */
        queue->sentinel[bucket] = node;
#if (CONFIG_PRIORITY_BUCKETS != 1)
        BITMAP_SET(queue->bitmap, bucket);
        								/* Mark the bucket list as used.      */
#endif
    } else {
#if (CONFIG_PRIORITY_BUCKETS != CONFIG_PRIORITY_LEVELS)
        nbias_list_sort_insert(queue->sentinel[bucket], node);
        								/* Priority search and insertion.  	  */
#else
        nbias_list_fifo_insert(queue->sentinel[bucket], node);
        							    /* FIFO insertion.                    */
#endif
    }
}



PORT_C_INLINE
void nprio_queue_remove(
    struct nprio_queue *        queue,
    struct nbias_list  *        node)
{
    uint_fast8_t                bucket;

#if (CONFIG_PRIORITY_BUCKETS != 1)
    bucket = nbias_list_get_bias(node) >> NPRIO_ARRAY_BUCKET_BITS;
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
void nprio_queue_rotate(
    struct nprio_queue *        queue,
    struct nbias_list  *        node)
{
    uint_fast8_t                bucket;

#if (CONFIG_PRIORITY_BUCKETS != 1)
    bucket = nbias_list_get_bias(node) >> NPRIO_ARRAY_BUCKET_BITS;
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
struct nbias_list * nprio_queue_peek(
    const struct nprio_queue *  queue)
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
bool nprio_queue_is_empty(
    const struct nprio_queue *  queue)
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



static void sched_init(
    struct sched_ctx *          ctx)
{
    ctx->current = NULL;
    nprio_queue_init(&ctx->run_queue);     /* Initialize run_queue structure. */
}



static void sched_term(
    struct sched_ctx *          ctx)
{
    ctx->current = NULL;
}



static struct nbias_list * sched_get_current(
    const struct sched_ctx *    ctx)
{
    return (ctx->current);
}



static void sched_insert_i(
    struct sched_ctx *          ctx,
    struct nbias_list *         thread_node)
{
    nprio_queue_insert(&ctx->run_queue, thread_node);
}



static void sched_remove_i(
    struct sched_ctx *          ctx,
    struct nbias_list *         thread_node)
{
    nprio_queue_remove(&ctx->run_queue, thread_node);
}



static struct nbias_list * sched_fetch_i(
    struct sched_ctx *          ctx)
{
    if (!nprio_queue_is_empty(&ctx->run_queue)) {
        struct nbias_list *     new_node;

        new_node = nprio_queue_peek(&ctx->run_queue);

        return (new_node);
    } else {

        return (NULL);
    }
}



#if   (CONFIG_PREEMPT == 1)
static struct nbias_list * sched_fetch_masked_i(
    struct sched_ctx *          ctx,
    uint_fast8_t                mask)
{
    struct nbias_list *         new_node;
    uint_fast8_t                new_prio;

    new_node = nprio_queue_peek(&ctx->run_queue);
    new_prio = (uint_fast8_t)nbias_list_get_bias(new_node);

    if (new_prio > mask) {

        return (new_node);
    } else {

        return (NULL);
    }
}
#endif



static void sched_run_i(
    struct sched_ctx *          ctx,
    struct nsys_lock *          lock)
{
    struct nbias_list *         new_node;

    while ((new_node = sched_fetch_i(ctx)) != NULL) {
        struct nthread *        new_thread;

        nprio_queue_rotate(&ctx->run_queue, new_node);
        ctx->current = new_node;
        new_thread   = NODE_TO_THREAD(new_node);
        nsys_lock_exit(lock);
        new_thread->entry(new_thread->stack);
        nsys_lock_enter(lock);
    }
}



#if   (CONFIG_PREEMPT == 1)
static void sched_preempt_i(
    struct sched_ctx *          ctx,
    struct nsys_lock *          lock)
{
    struct nbias_list *         new_node;
    struct nbias_list *         old_node;
    uint_fast8_t                mask;

    old_node = ctx->current;
    mask     = (uint_fast8_t)nbias_list_get_bias(old_node);

    while ((new_node = sched_fetch_masked_i(ctx, mask)) != NULL) {
        struct nthread *        new_thread;

        nprio_queue_rotate(&ctx->run_queue, new_node);
        ctx->current = new_node;
        new_thread   = NODE_TO_THREAD(new_node);
        nsys_lock_exit(lock);
        new_thread->entry(new_thread->stack);
        nsys_lock_enter(lock);
    }
    ctx->current = old_node;
}
#endif

/*===================================  GLOBAL PRIVATE FUNCTION DEFINITIONS  ==*/
/*====================================  GLOBAL PUBLIC FUNCTION DEFINITIONS  ==*/


void nkernel_init(void)
{
    nsys_lock_init();
    sched_init(&g_domain.sched);
}



void nkernel_term(void)
{
    sched_term(&g_domain.sched);
    nsys_lock_term();
}



void nkernel_start(void)
{
    struct nsys_lock            lock;

    nsys_lock_enter(&lock);
    sched_run_i(&g_domain.sched, &lock);
    nsys_lock_exit(&lock);
}



void nkernel_isr_enter(void)
{
#if   (CONFIG_PREEMPT == 1)
    nisr_enter();
#endif
}



void nkernel_isr_exit(void)
{
#if   (CONFIG_PREEMPT == 1)
    nisr_exit();
#endif
}



void nkernel_schedule_i(
    struct nsys_lock *          lock)
{
#if   (CONFIG_PREEMPT == 1)
    if (!nisr_is_active()) {
        sched_preempt_i(&g_domain.sched, lock);
    } else {
        nisr_pend_kernel();
    }
#else
    (void)lock;
#endif
}



void nthread_init(
    struct nthread *            thread,
    void                     (* entry)(void *),
    void *                      stack,
    uint_fast8_t                priority)
{
    NREQUIRE(NAPI_POINTER, thread != NULL);
    NREQUIRE(NAPI_OBJECT,  thread->signature != THREAD_SIGNATURE);
    NREQUIRE(NAPI_POINTER, entry  != NULL);
    NREQUIRE(NAPI_POINTER, stack  != NULL);
    NREQUIRE(NAPI_RANGE,   priority < CONFIG_BIAS_LEVELS);
    NOBLIGATION(thread->signature = THREAD_SIGNATURE);  /* Validate structure */

    thread->entry = entry;
    thread->stack = stack;
    nbias_list_init(&thread->queue_node, priority);
#if   (CONFIG_REGISTRY == 1)
    thread->name  = NULL;
    ndlist_init(&thread->registry_node);
#endif
}



void nthread_term(void)
{
    struct nsys_lock            lock;
    struct nthread *            thread;

    nsys_lock_enter(&lock);
    thread = NODE_TO_THREAD(sched_get_current(&g_domain.sched));

    if (thread->ref != 0u) {
        thread->ref  = 0u;
        sched_remove_i(&g_domain.sched, &thread->queue_node);
    }
    nsys_lock_exit(&lock);
    NOBLIGATION(thread->signature = ~THREAD_SIGNATURE);
}



struct nthread * nthread_get_current(void)
{
    struct nthread *            current_thread;

    current_thread = NODE_TO_THREAD(sched_get_current(&g_domain.sched));

    NREQUIRE_INTERNAL(NAPI_OBJECT, current_thread->signature == THREAD_SIGNATURE);

    return (current_thread);
}



void nthread_ready_i(
    struct nthread *            thread)
{
    NREQUIRE(NAPI_OBJECT, thread->signature == THREAD_SIGNATURE);

    ncpu_sat_increment(&thread->ref);

    if (thread->ref == 1u) {
        sched_insert_i(&g_domain.sched, &thread->queue_node);
    }
}



void nthread_block_i(
    struct nthread *            thread)
{
    NREQUIRE(NAPI_POINTER, thread != NULL);
    NREQUIRE(NAPI_OBJECT,  thread->signature == THREAD_SIGNATURE);

    if (thread->ref == 1u) {
        sched_remove_i(&g_domain.sched, &thread->queue_node);
    }
    ncpu_sat_decrement(&thread->ref);
}



void nthread_sleep_i(void)
{
    nthread_block_i(nthread_get_current());
}



uint_fast8_t nthread_get_priority(void)
{
    struct nthread *            thread;

    thread = nthread_get_current();

    return ((uint_fast8_t)nbias_list_get_bias(&thread->queue_node));
}



void nthread_set_priority(
    uint_fast8_t                priority)
{
    struct nthread *            thread;
    struct nsys_lock            lock;

    NREQUIRE(NAPI_RANGE,  priority < CONFIG_BIAS_LEVELS);

    thread = nthread_get_current();
    nsys_lock_enter(&lock);
    sched_remove_i(&g_domain.sched, &thread->queue_node);
    nbias_list_set_bias(&thread->queue_node, priority);
    sched_insert_i(&g_domain.sched, &thread->queue_node);
    nsys_lock_exit(&lock);
}

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//** @} *//*********************************************
 * END of nthread.c
 ******************************************************************************/
