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
#include "arch/cpu.h"
#include "arch/intr.h"
#include "arch/systimer.h"
#include "lib/ndebug.h"
#include "lib/nprio_queue.h"
#include "nkernel.h"

/*=========================================================  LOCAL MACRO's  ==*/

/**@brief       Thread structure signature.
 * @details     The signature is used to confirm that a structure passed to a
 *              function is indeed a nthread thread structure.
 */
#define THREAD_SIGNATURE                    ((n_native)0xfeedbeeful)

#define NODE_TO_THREAD(node)                                                    \
    CONTAINER_OF(node, struct nthread, queue_node)

/*======================================================  LOCAL DATA TYPES  ==*/


/**@brief       Scheduler context structure
 * @details     This structure holds important status data for the scheduler.
 */
struct sched_ctx
{
    struct nthread *            current;                                        /**<@brief The current thread         */
    struct nprio_queue          run_queue;                                      /**<@brief Run queue of threads       */
};


/**@brief       System context structure
 * @details     This structure holds all local data for the whole system. If the
 *              kernel is used in preemptable environment then this structure
 *              will be allocated one per thread. If the underlying OS has fast
 *              synchronization mechanism then all critical sections can be
 *              protected by using the said mechanism.
 */
struct sys_ctx
{
#if (NSYS_LOCK_USES_RESOURCE == 1)
    NSYS_LOCK_DECL_RESOURCE(lock_res);
#endif
    struct sched_ctx            sched;
};

/*=============================================  LOCAL FUNCTION PROTOTYPES  ==*/


static PORT_C_INLINE struct sys_ctx * sys_context_get(
    const struct nthread *      thread);
    
    
    
static PORT_C_INLINE void sys_context_init(
    struct nthread *            thread);
    
    
    
static PORT_C_INLINE void sched_init(
    void);



static PORT_C_INLINE struct nbias_list * sched_get_current(
    void);



static PORT_C_INLINE void sched_insert_i(
    struct nbias_list *         thread_node);



static PORT_C_INLINE void sched_remove_i(
    struct nbias_list *         thread_node);



static PORT_C_INLINE void sched_schedule(
    void);

/*=======================================================  LOCAL VARIABLES  ==*/

/**@brief       Provides the basic information about this module
 */
static const NMODULE_INFO_CREATE("Neon RT Kernel", "Nenad Radulovic");

/*======================================================  GLOBAL VARIABLES  ==*/
/*============================================  LOCAL FUNCTION DEFINITIONS  ==*/


static PORT_C_INLINE struct sys_ctx * sys_context_get(
    const struct nthread *      thread)
{
#if (CONFIG_SYS_PREEMPT_AWARE == 0)
    static struct sys_ctx       sys_context;

    (void)thread;
        
    return (&sys_context);
#else
    if (thread != NULL) {
        return (thread->sys_context);
    } else {
        return (callback_get_tls());
    }
#endif
}



static PORT_C_INLINE void sys_context_init(
    struct nthread *            thread)
{
#if (CONFIG_SYS_PREEMPT_AWARE == 0)
    (void)thread;
#else
    thread->sys_context = callback_get_tls();
#endif
}



static PORT_C_INLINE void sched_init(
    struct sched_ctx *          ctx)
{
    ctx->current = NULL;
    nprio_queue_init(&ctx->run_queue);                                          /* Initialize run_queue structure.    */
}



static PORT_C_INLINE struct nbias_list * sched_get_current(
    struct sched_ctx *          ctx)
{
    return (&ctx->current->queue_node);
}



static PORT_C_INLINE void sched_insert_i(
    struct sched_ctx *          ctx,
    struct nbias_list *         thread_node)
{
    nprio_queue_insert(&ctx->run_queue, thread_node);
}



static PORT_C_INLINE void sched_remove_i(
    struct sched_ctx *          ctx,
    struct nbias_list *         thread_node)
{
    nprio_queue_remove(&ctx->run_queue, thread_node);
}



static PORT_C_INLINE void sched_schedule(
    struct sched_ctx *          ctx)
{
    lock_ctx                    lock;

    nsys_lock(&lock);

    while (!nprio_queue_is_empty(&ctx->run_queue)) {
        struct nbias_list *     new_node;
        struct nthread *        new_thread;

        new_node     = nprio_queue_peek(&ctx->run_queue);
        new_thread   = NODE_TO_THREAD(new_node);
#if (CONFIG_HOOK_THREAD_SWITCH == 1)
        hook_on_thread_switch(ctx->current, new_thread);
#endif
        ctx->current = new_thread;
        nsys_unlock(&lock);
        ctx->current->entry(ctx->current->stack);
        nsys_lock(&lock);
        nprio_queue_rotate(&ctx->run_queue, new_node);
    }
    ctx->current = NULL;
    nsys_unlock(&lock);
}

/*===================================  GLOBAL PRIVATE FUNCTION DEFINITIONS  ==*/
/*====================================  GLOBAL PUBLIC FUNCTION DEFINITIONS  ==*/


void nkernel_init(void)
{
    ncpu_module_init();
    nintr_module_init();
    ntimer_module_init();
    NSYS_LOCK_INIT(&sys_context_get(NULL)->lock_res);
    sched_init();

#if (CONFIG_HOOK_SYS_INIT == 1)
    hook_on_sys_init();
#endif
}



void nkernel_term(void)
{
    ntimer_module_term();
    nintr_module_term();
    ncpu_module_term();
}



void nkernel_start(void)
{
#if (CONFIG_HOOK_SYS_START == 1)
    hook_on_sys_start();
#endif
    sched_schedule();
}



size_t nkernel_get_context_size(void)
{
    return (sizeof(struct sys_ctx));
}



void nsys_lock(
    lock_ctx *                  lock)
{
    NSYS_LOCK_ENTER(lock, &sys_context_get(NULL)->lock_res);
}



void nsys_unlock(
    lock_ctx *                  lock)
{
    NSYS_LOCK_EXIT(lock, &sys_context_get(NULL)->lock_res);
}



void nthread_init(
    struct nthread *            thread,
    void                     (* entry)(void *),
    void *                      stack,
    uint_fast8_t                priority)
{
    lock_ctx                    lock;

    NREQUIRE(NAPI_POINTER, thread != NULL);
    NREQUIRE(NAPI_OBJECT,  thread->signature != THREAD_SIGNATURE);
    NREQUIRE(NAPI_POINTER, entry  != NULL);
    NREQUIRE(NAPI_POINTER, stack  != NULL);
    NREQUIRE(NAPI_RANGE,   priority < CONFIG_PRIORITY_LEVELS);
    NOBLIGATION(thread->signature = THREAD_SIGNATURE);                          /* Validate thread structure          */

    thread->entry = entry;
    thread->stack = stack;
    nbias_list_init(&thread->queue_node, priority);
    sys_context_init(thread);
#if   (CONFIG_REGISTRY          == 1)
    thread->name  = NULL;
    ndlist_init(thread->registry_node);
#endif
    nsys_lock(&lock);
    sched_insert_i(&thread->queue_node);                                        /* Add to Run Queue                   */
    nsys_unlock(&lock);

#if   (CONFIG_HOOK_THREAD_INIT == 1u)
    hook_on_thread_init(thread);
#endif
}



void nthread_term(
    void)
{
    struct nthread *            thread;
    lock_ctx                    lock;

    thread = nthread_get_current();
#if (CONFIG_HOOK_AT_THREAD_TERM == 1)
    hook_on_thread_term(thread);
#endif
    nsys_lock(&lock);
    nthread_block_i(thread);
    nsys_unlock(&lock);
    NOBLIGATION(thread->signature = ~THREAD_SIGNATURE);
}



struct nthread * nthread_get_current(void)
{
    struct sched_ctx *          ctx = &sys_context_get(NULL)->sched;
    
    return (NODE_TO_THREAD(sched_get_current(ctx)));
}



void nthread_ready_i(
    struct nthread *            thread)
{
    NREQUIRE(NAPI_OBJECT, thread->signature == THREAD_SIGNATURE);

    thread->ref++;

    if (thread->ref == 1u) {
        struct sched_ctx *      ctx = &sys_context_get(thread)->sched;
        
        sched_insert_i(ctx, &thread->queue_node);
    }
}



void nthread_block_i(
    struct nthread *            thread)
{
    NREQUIRE(NAPI_POINTER, thread != NULL);
    NREQUIRE(NAPI_OBJECT,  thread->signature == THREAD_SIGNATURE);

    thread->ref--;

    if (thread->ref == 0u) {
        struct sched_ctx *      ctx = &sys_context_get(thread)->sched;
        
        sched_remove_i(ctx, &thread->queue_node);
    }
}



void nthread_sleep_i(
    void)
{
    nthread_block_i(nthread_get_current());
}



uint_fast8_t nthread_get_priority(
    void)
{
    struct nthread *            thread;

    thread = nthread_get_current();
    
    NREQUIRE_INTERNAL(NAPI_OBJECT, thread->signature == THREAD_SIGNATURE);

    return ((uint_fast8_t)nbias_list_get_bias(&thread->queue_node));
}



void nthread_set_priority(
    uint_fast8_t                priority)
{
    struct nthread *            thread;
    struct sched_ctx *          ctx;
    lock_ctx                    lock;

    thread = nthread_get_current();
    ctx    = &sys_context_get(thread)->sched;
    
    NREQUIRE(NAPI_RANGE,  priority < CONFIG_PRIORITY_LEVELS);
    NREQUIRE_INTERNAL(NAPI_OBJECT, thread->signature == THREAD_SIGNATURE);

    nsys_lock(&lock);
    sched_remove_i(ctx, &thread->queue_node);
    nbias_list_set_bias(&thread->queue_node, priority);
    sched_insert_i(ctx, &thread->queue_node);
    nsys_unlock(&lock);
}

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//** @} *//*********************************************
 * END of nthread.c
 ******************************************************************************/
