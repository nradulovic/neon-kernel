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
 * @brief       Semaphore implementation
 * @addtogroup  semaphore
 *********************************************************************//** @{ */
/**@defgroup    semaphore_impl Implementation
 * @brief       Implementation
 * @{ *//*--------------------------------------------------------------------*/

/*=========================================================  INCLUDE FILES  ==*/

#include "plat/critical.h"
#include "arch/intr.h"
#include "kernel/nsemaphore.h"
#include "kernel/nthread.h"
#include "kernel/nsched.h"

/*=========================================================  LOCAL MACRO's  ==*/

/**@brief       Timer structure signature.
 * @details     The signature is used to confirm that a structure passed to a
 *              semaphore function is indeed a semaphore structure.
 */
#define SEM_SIGNATURE                       ((n_native)0xfeedbef0ul)

/*======================================================  LOCAL DATA TYPES  ==*/
/*=============================================  LOCAL FUNCTION PROTOTYPES  ==*/
/*=======================================================  LOCAL VARIABLES  ==*/
/*======================================================  GLOBAL VARIABLES  ==*/
/*============================================  LOCAL FUNCTION DEFINITIONS  ==*/
/*===================================  GLOBAL PRIVATE FUNCTION DEFINITIONS  ==*/
/*====================================  GLOBAL PUBLIC FUNCTION DEFINITIONS  ==*/

void nsem_init(
    struct nsem *               sem,
    int32_t                     count)
{
    nprio_queue_init(&sem->prio_array);
    sem->count = count;
}



void nsem_term(
    struct nsem *               sem)
{
    nintr_ctx                   intr_ctx;

    NCRITICAL_LOCK_ENTER(&intr_ctx);

    while (!nprio_queue_is_empty(&sem->prio_array)) {
        struct nbias_list *     current_node;

        current_node = nprio_queue_peek(&sem->prio_array);
        nthread_from_queue_node(current_node)->status = N_E_OBJ_REMOVED;
        nprio_queue_remove(&sem->prio_array, current_node);
        nsched_insert_i(current_node);
    }
    nsched_reschedule_i();
    NCRITICAL_LOCK_EXIT(intr_ctx);
}



enum n_status nsem_wait(
    struct nsem *               sem)
{
    nintr_ctx                   intr_ctx;

    NCRITICAL_LOCK_ENTER(&intr_ctx);
    sem->count--;

    if (sem->count < 0) {
        struct nbias_list *     current_node;

        current_node = nsched_remove_current_i();
        nprio_queue_insert(&sem->prio_array, current_node);
        nsched_reschedule_i();
        NCRITICAL_LOCK_EXIT(intr_ctx);

        return (nthread_from_queue_node(current_node)->status);
    } else {
        NCRITICAL_LOCK_EXIT(intr_ctx);

        return (N_SUCCESS);
    }
}



void nsem_signal(
    struct nsem *               sem)
{
    nintr_ctx                   intr_ctx;

    NCRITICAL_LOCK_ENTER(&intr_ctx);
    sem->count++;
}

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//** @} *//*********************************************
 * END of semaphore.c
 ******************************************************************************/
