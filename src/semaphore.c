/*
 * This file is part of eSolid.
 *
 * Copyright (C) 2010 - 2013 Nenad Radulovic
 *
 * eSolid is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * eSolid is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with eSolid.  If not, see <http://www.gnu.org/licenses/>.
 *
 * web site:    http://github.com/nradulovic
 * e-mail  :    nenad.b.radulovic@gmail.com
 *//***********************************************************************//**
 * @file
 * @author      Nenad Radulovic
 * @brief       Short desciption of file
 * @addtogroup  sem_impl
 *********************************************************************//** @{ */

/*=========================================================  INCLUDE FILES  ==*/

#include "kernel/semaphore.h"

/*=========================================================  LOCAL MACRO's  ==*/
/*======================================================  LOCAL DATA TYPES  ==*/
/*=============================================  LOCAL FUNCTION PROTOTYPES  ==*/
/*=======================================================  LOCAL VARIABLES  ==*/
/*======================================================  GLOBAL VARIABLES  ==*/
/*============================================  LOCAL FUNCTION DEFINITIONS  ==*/
/*===================================  GLOBAL PRIVATE FUNCTION DEFINITIONS  ==*/
/*====================================  GLOBAL PUBLIC FUNCTION DEFINITIONS  ==*/

void nsem_init(
    struct nsem *               sem,
    uint32_t                    count)
{
    NDLIST_INIT(list, sem);
    nprio_array_init(&sem->prio_array);
    sem->count = count;
}

void nsem_term(
    struct nsem *               sem);

#define PORT_LOAD(val)          *(val)
#define PORT_SAVE(address, val) *(address) = val

void nsem_wait(
    struct nsem *               sem)
{
}

/**@brief       Wait on a semaphore
 * @param       sem
 *              Semaphore: points to a semaphore object to initialize.
 * @param       time
 *              Time: the timeout time specified in system ticks.
 */
void nsem_wait_timeout(
    struct nsem *               sem,
    esVTmrTick                  time);

/**@brief       Increment the value of a semaphore
 * @param       sem
 *              Semaphore: points to a semaphore object to initialize.
 */
void nsem_post(
    struct nsem *               sem);


/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of semaphore.c
 ******************************************************************************/
