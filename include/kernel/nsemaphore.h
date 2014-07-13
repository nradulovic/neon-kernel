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
 * @brief       Semaphore header
 * @defgroup    semaphore Semaphore
 * @brief       Semaphore
 *********************************************************************//** @{ */
/**@defgroup    semaphore_intf Interface
 * @brief       Public interface
 * @{ *//*--------------------------------------------------------------------*/

#ifndef SEMAPHORE_H
#define SEMAPHORE_H

/*=========================================================  INCLUDE FILES  ==*/

#include "plat/compiler.h"
#include "kernel/nub_config.h"
#include "lib/nprio_array.h"
#include "lib/nstatus.h"

/*===============================================================  MACRO's  ==*/
/*------------------------------------------------------  C++ extern begin  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/

/**@brief       Semaphore structure
 * @api
 */
struct nsem {
    struct nprio_array          prio_array;
    int32_t                     count;
#if   (CONFIG_DEBUG_API == 1u) || defined(__DOXYGEN__)
    n_native                    signature;                                      /**<@brief Debug signature            */
#endif
};

/**@brief       Semaphore type
 * @api
 */
typedef struct nsem nsem;

/*======================================================  GLOBAL VARIABLES  ==*/
/*===================================================  FUNCTION PROTOTYPES  ==*/

/**@brief       Initialize a semaphore
 * @param       sem
 *              Semaphore: points to a semaphore object to initialize.
 * @param       count
 *              Count: is an initial value to set the semaphore to.
 */
void nsem_init(
    struct nsem *               sem,
    int32_t                     count);



void nsem_term(
    struct nsem *               sem);



/**@brief       Wait on a semaphore
 * @param       sem
 *              Semaphore: points to a semaphore object to initialize.
 */
enum n_status nsem_wait(
    struct nsem *               sem);



/**@brief       Wait on a semaphore
 * @param       sem
 *              Semaphore: points to a semaphore object to initialize.
 * @param       time
 *              Time: the timeout time specified in system ticks.
 */


/**@brief       Increment the value of a semaphore
 * @param       sem
 *              Semaphore: points to a semaphore object to initialize.
 */
void nsem_signal(
    struct nsem *               sem);

/*--------------------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//** @} *//*********************************************
 * END of nsemaphore.h
 ******************************************************************************/
#endif /* SEMAPHORE_H */
