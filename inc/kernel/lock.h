/*
 * This file is part of eSolid - RT Kernel
 *
 * Copyright (C) 2011, 2012, 2013 - Nenad Radulovic
 *
 * eSolid - RT Kernel is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option) any
 * later version.
 *
 * eSolid - RT Kernel is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * eSolid - RT Kernel; if not, write to the Free Software Foundation, Inc., 51
 * Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * web site:    http://github.com/nradulovic
 * e-mail  :    nenad.b.radulovic@gmail.com
 *//***********************************************************************//**
 * @file
 * @author  	Nenad Radulovic
 * @brief       Critical code and kernel lock management
 * @addtogroup  kern_lock_intf
 *********************************************************************//** @{ */

#if !defined(LOCK_H__)
#define LOCK_H__

/*=========================================================  INCLUDE FILES  ==*/

#include "arch/cpu.h"

/*===============================================================  MACRO's  ==*/

/**@} *//*----------------------------------------------------------------*//**
 * @name        Critical code management
 * @brief       Disable/enable interrupts by preserving the interrupt context
 * @details     Generally speaking these macros would store the interrupt
 *              context in the local variable of portReg_T type and then disable
 *              interrupts. Local variable is allocated in all of eSolid-Kernel
 *              functions that need to disable interrupts. Macros would restore
 *              the interrupt context by copying back the allocated variable
 *              into the interrupt controller status/control register.
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Enter critical code section
 * @param       intCtx
 *              Interrupt context, pointer to portable type variable which will
 *              hold the interrupt context state during the critical code
 *              section.
 */
#define ES_CRITICAL_LOCK_ENTER(intCtx)                                          \
    PORT_INT_PRIO_REPLACE(intCtx, PORT_DEF_MAX_ISR_PRIO)

/**@brief       Exit critical code section
 * @param       intCtx
 *              Interrupt context, portable type variable which is holding a
 *              previously saved interrupt context state.
 */
#define ES_CRITICAL_LOCK_EXIT(intCtx)                                           \
    PORT_INT_PRIO_SET(intCtx)

/**@} *//*----------------------------------------------  C++ extern begin  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/

/**@} *//*----------------------------------------------------------------*//**
 * @name        Critical code section locking management
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Kernel lock context type
 * @details     Variables declared using this type can hold current lock context
 *              which can be restored after a critical code section is exited.
 */
typedef portReg_T esLockCtx_T;

/** @} *//*-------------------------------------------------------------------*/

/*======================================================  GLOBAL VARIABLES  ==*/
/*===================================================  FUNCTION PROTOTYPES  ==*/

/**@} *//*----------------------------------------------------------------*//**
 * @name        Multi-threading locking management
 * @details     These methods are often used to protect concurrent access to a
 *              protected resource.
 *
 *              For more details see @ref critical_section.
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Enter a critical code lock
 * @param       lockCtx
 *              Pointer to context variable where to store the current lock
 *              context.
 * @called
 * @fromapp
 * @fromthd
 * @schedno
 * @api
 */
void esKernLockIntEnter(
    esLockCtx_T *       lockCtx);

/**@brief       Exit a critical code lock
 * @param       lockCtx
 *              Context variable value
 * @details     Restores the lock context to state before the
 *              esKernLockIntEnter() was called.
 * @called
 * @fromapp
 * @fromthd
 * @schedmaybe
 * @api
 */
void esKernLockIntExit(
    esLockCtx_T         lockCtx);

/**@brief       Lock the scheduler
 * @pre         1) `The kernel state < ES_KERN_INIT`, see @ref states.
 * @called
 * @fromthd
 * @schedno
 * @iclass
 */
void esKernLockEnterI(
    void);

/**@brief       Unlock the scheduler
 * @pre         1) `The kernel state < ES_KERN_INIT`, see @ref states.
 * @pre         2) `gKernLockCnt > 0u`, current number of locks must be greater
 *                  than zero, in other words: each call to kernel lock function
 *                  must have its matching call to kernel unlock function.
 * @called
 * @fromthd
 * @schedmaybe
 * @iclass
 */
void esKernLockExitI(
    void);

/**@brief       Lock the scheduler
 * @pre         1) `The kernel state < ES_KERN_INIT`, see @ref states.
 * @called
 * @fromthd
 * @schedno
 * @api
 */
void esKernLockEnter(
    void);

/**@brief       Unlock the scheduler
 * @pre         1) `The kernel state < ES_KERN_INIT`, see @ref states.
 * @pre         2) `gKernLockCnt > 0u`, current number of locks must be greater
 *                  than zero, in other words: each call to kernel lock function
 *                  must have its matching call to kernel unlock function.
 * @called
 * @fromthd
 * @schedmaybe
 * @api
 */
void esKernLockExit(
    void);

/** @} *//*-----------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of lock.h
 ******************************************************************************/
#endif /* LOCK_H__ */
