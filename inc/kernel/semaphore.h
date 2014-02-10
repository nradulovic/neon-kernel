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
 * @author  	Nenad Radulovic
 * @brief       Interface of semaphore.
 * @details     Detailed description
 * @addtogroup  sem_intf
 *********************************************************************//** @{ */

#if !defined(SEMAPHORE_H_)
#define SEMAPHORE_H_

/*=========================================================  INCLUDE FILES  ==*/

#include "kernel/kernel.h"

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

/*------------------------------------------------------------------------*//**
 * @name        Data types list
 * @brief       brief description
 * @{ *//*--------------------------------------------------------------------*/
#define CFG_SEMAPHORE_CNT_T             int16_t

typedef CFG_SEMAPHORE_CNT_T esSemCnt_T;

struct esSem {
    esThreadQ        thds;
    esSemCnt_T      cnt;
};

typedef struct esSem esSem_T;

/** @} *//*-------------------------------------------------------------------*/
/*======================================================  GLOBAL VARIABLES  ==*/
/*===================================================  FUNCTION PROTOTYPES  ==*/

/*------------------------------------------------------------------------*//**
 * @name        Function list
 * @brief       brief description
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Initialize a semaphore
 * @param       sem
 *              Semaphore: points to a semaphore object to initialize.
 * @param       cnt
 *              Count: is an initial value to set the semaphore to.
 */
void esSemInit(
    esSem_T *       sem,
    esSemCnt_T      cnt);


void esSemTerm(
    esSem_T *       sem);

/**@brief       Wait on a semaphore
 * @param       sem
 *              Semaphore: points to a semaphore object to initialize.
 */
void esSemWait(
    esSem_T *       sem);

/**@brief       Wait on a semaphore
 * @param       sem
 *              Semaphore: points to a semaphore object to initialize.
 * @param       time
 *              Time: the timeout time specified in system ticks.
 */
void esSemWaitTimeout(
    esSem_T *       sem,
    esVTmrTick      time);

/**@brief       Increment the value of a semaphore
 * @param       sem
 *              Semaphore: points to a semaphore object to initialize.
 */
void esSemPost(
    esSem_T *       sem);

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
