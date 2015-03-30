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

#include "port/core.h"
#include "shared/component.h"
#include "shared/bitop.h"
#include "sched/prio_queue.h"
#include "sched/sched.h"

/*=========================================================  LOCAL MACRO's  ==*/

#define NODE_TO_THREAD(node_ptr)                                                \
    CONTAINER_OF(node_ptr, struct nthread, node)

/*======================================================  LOCAL DATA TYPES  ==*/

/**@brief       Scheduler context structure
 * @details     This structure holds important status data for the scheduler.
 */
struct sched_ctx
{
    struct nbias_list *         current;    /**<@brief The current thread     */
    struct nprio_queue          run_queue;  /**<@brief Run queue of threads   */
};

/*=============================================  LOCAL FUNCTION PROTOTYPES  ==*/
/*=======================================================  LOCAL VARIABLES  ==*/

static struct sched_ctx         g_sched_ctx;

/*======================================================  GLOBAL VARIABLES  ==*/
/*============================================  LOCAL FUNCTION DEFINITIONS  ==*/
/*===================================  GLOBAL PRIVATE FUNCTION DEFINITIONS  ==*/
/*====================================  GLOBAL PUBLIC FUNCTION DEFINITIONS  ==*/

void nsched_init(void)
{
    struct sched_ctx *          ctx = &g_sched_ctx;

    ctx->current = NULL;
    nprio_queue_init(&ctx->run_queue);     /* Initialize run_queue structure. */
}



void nmodule_sched_term(void)
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
    memset(thread->name, 0, sizeof(thread->name));

    if (define->name) {
        strncpy(thread->name, define->name, sizeof(thread->name));
    }
    ndlist_init(&thread->registry_node);
#endif
}



void nsched_thread_term(struct nthread * thread)
{
    struct sched_ctx *          ctx = &g_sched_ctx;
    ncore_lock                   sys_lock;

    ncore_lock_enter(&sys_lock);

    if (thread->ref != 0u) {
        thread->ref =  0u;
        nprio_queue_remove(&ctx->run_queue, &thread->node);
    }
    nbias_list_term(&thread->node);
    ncore_lock_exit(&sys_lock);
}



void nsched_thread_insert_i(struct nthread * thread)
{
    ncore_sat_increment(&thread->ref);

    if (thread->ref == 1u) {
        struct sched_ctx *      ctx = &g_sched_ctx;

        nprio_queue_insert(&ctx->run_queue, &thread->node);
    }
}



void nsched_thread_remove_i(struct nthread * thread)
{
    if (thread->ref == 1u) {
        struct sched_ctx *      ctx = &g_sched_ctx;

        nprio_queue_remove(&ctx->run_queue, &thread->node);
    }
    ncore_sat_decrement(&thread->ref);
}



struct nthread * nsched_thread_fetch_i(void)
{
    struct sched_ctx *          ctx = &g_sched_ctx;
    struct nbias_list *         new_node;

    if (!nprio_queue_is_empty(&ctx->run_queue)) {
        new_node = nprio_queue_peek(&ctx->run_queue);
        nprio_queue_rotate(&ctx->run_queue, new_node);
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
 * END of sched.c
 ******************************************************************************/
