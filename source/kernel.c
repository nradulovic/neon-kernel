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

#include "nkernel_config.h"

#include "plat/compiler.h"
#include "plat/sys_lock.h"
#include "arch/ncore.h"
#include "lib/ndebug.h"
#include "lib/nlist.h"
#include "lib/nbias_list.h"
#include "lib/nprio_queue.h"
#include "nkernel.h"

/*=========================================================  LOCAL MACRO's  ==*/

/**@brief       Thread structure signature.
 * @details     The signature is used to confirm that a structure passed to a
 *              function is indeed a nthread thread structure.
 */
#define THREAD_SIGNATURE                    ((ncpu_reg)0xfeedbeeful)

#define NODE_TO_THREAD(node)                                                    \
    CONTAINER_OF(node, struct nthread, queue_node)

/*======================================================  LOCAL DATA TYPES  ==*/

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
 *              synchronization mechanism then all critical sections can be
 *              protected by using the said mechanism.
 */
struct sys_domain
{
    struct sched_ctx            sched;
};

/*=============================================  LOCAL FUNCTION PROTOTYPES  ==*/


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
static const NMODULE_INFO_CREATE("Neon RT Kernel", "Nenad Radulovic");

static struct sys_domain        g_domain;

/*======================================================  GLOBAL VARIABLES  ==*/
/*============================================  LOCAL FUNCTION DEFINITIONS  ==*/


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
    ncore_init();
    nsys_lock_init();
    sched_init(&g_domain.sched);
}



void nkernel_term(void)
{
    sched_term(&g_domain.sched);
    nsys_lock_term();
    ncore_term();
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
    NREQUIRE(NAPI_RANGE,   priority < CONFIG_PRIORITY_LEVELS);
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

    NREQUIRE(NAPI_RANGE,  priority < CONFIG_PRIORITY_LEVELS);
        
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
