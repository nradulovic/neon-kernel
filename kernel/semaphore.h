/*
 * This file is part of eSolid-Kernel
 *
 * Copyright (C) 2013 - Nenad Radulovic
 *
 * eSolid-Kernel is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * eSolid-Kernel is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with eSolid-Kernel; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 *
 * web site:    http://blueskynet.dyndns-server.com
 * e-mail  :    blueskyniss@gmail.com
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
#include "kernel.h"

/*===============================================================  MACRO's  ==*/

/*------------------------------------------------------------------------*//**
 * @name        Macro group
 * @brief       brief description
 * @{ *//*--------------------------------------------------------------------*/

/** @} *//*-------------------------------------------------------------------*/
/*------------------------------------------------------  C++ extern begin  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/

/*------------------------------------------------------------------------*//**
 * @name        Data types group
 * @brief       brief description
 * @{ *//*--------------------------------------------------------------------*/
#define CFG_SEMAPHORE_CNT_T             int16_t

typedef CFG_SEMAPHORE_CNT_T esSemCnt_T;

struct esSem {
    esThdQ_T        thds;
    esSemCnt_T      cnt;
};

typedef struct esSem esSem_T;

/** @} *//*-------------------------------------------------------------------*/
/*======================================================  GLOBAL VARIABLES  ==*/
/*===================================================  FUNCTION PROTOTYPES  ==*/

/*------------------------------------------------------------------------*//**
 * @name        Function group
 * @brief       brief description
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Initialize a semaphore
 * @param       sem
 *              Semaphore: points to a semaphore object to initialize.
 * @param       qm
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
    esTick_T      time);

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
