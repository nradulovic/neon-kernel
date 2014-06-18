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
 *//***********************************************************************//**
 * @file
 * @author  	Nenad Radulovic
 * @brief       Interface of semaphore.
 * @details     Detailed description
 * @addtogroup  sem_intf
 *********************************************************************//** @{ */

#if !defined(SEMAPHORE_H_)
#define SEMAPHORE_H_

/*=========================================================  INCLUDE FILES  ==*/

#include <stdint.h>
#include "kernel/nlist.h"
#include "kernel/nprio_array.h"

#include "kernel/nsys.h"

/*===============================================================  MACRO's  ==*/

/*------------------------------------------------------------------------*//**
 * @name        Macro list
 * @brief       brief description
 * @{ *//*--------------------------------------------------------------------*/

/** @} *//*-------------------------------------------------------------------*/
/*------------------------------------------------------  C++ extern begin  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/

struct nsem {
    struct nsem_list
    {
        struct nsem *               next;
        struct nsem *               prev;
    }                           list;
    struct nprio_array          prio_array;
    uint32_t                    count;
};

/*======================================================  GLOBAL VARIABLES  ==*/
/*===================================================  FUNCTION PROTOTYPES  ==*/

/*------------------------------------------------------------------------*//**
 * @name        Function list
 * @brief       brief description
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Initialize a semaphore
 * @param       sem
 *              Semaphore: points to a semaphore object to initialize.
 * @param       count
 *              Count: is an initial value to set the semaphore to.
 */
void nsem_init(
    struct nsem *               sem,
    uint32_t                    count);


void nsem_term(
    struct nsem *               sem);

/**@brief       Wait on a semaphore
 * @param       sem
 *              Semaphore: points to a semaphore object to initialize.
 */
void nsem_wait(
    struct nsem *               sem);

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

/** @} *//*-------------------------------------------------------------------*/
/*--------------------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of semaphore.h
 ******************************************************************************/
#endif /* SEMAPHORE_H_ */
