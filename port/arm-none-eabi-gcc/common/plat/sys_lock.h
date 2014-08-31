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
 * @author  	Nenad Radulovic
 * @brief       Interface of ARM Cortex critical code section port.
 * @addtogroup  arm-none-eabi-gcc
 *********************************************************************//** @{ */
/**@defgroup    arm-none-eabi-gcc-critical Critical code section
 * @brief       Critical code section
 * @{ *//*--------------------------------------------------------------------*/

#ifndef SYS_LOCK_H
#define SYS_LOCK_H

/*=========================================================  INCLUDE FILES  ==*/

#include "plat/compiler.h"
#include "arch/ncore.h"

/*===============================================================  MACRO's  ==*/

#define nsys_lock_init()                    (void)0

#define nsys_lock_term()                    (void)0

/*------------------------------------------------------  C++ extern begin  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/

struct nsys_lock
{
    nisr_ctx                    isr_ctx;
};

typedef struct nsys_lock nsys_lock;

/*======================================================  GLOBAL VARIABLES  ==*/
/*===================================================  FUNCTION PROTOTYPES  ==*/



/**@brief       Enter critical code section
 * @param       resource
 *              Interrupt resource, pointer to portable type variable which will
 *              hold the interrupt context state during the critical code
 *              section.
 */
PORT_C_INLINE
void nsys_lock_enter(
    struct nsys_lock *          lock)
{
    lock->isr_ctx = nisr_replace_mask(NISR_PRIO_TO_CODE(CONFIG_ISR_MAX_PRIO));
}



/**@brief       Exit critical code section
 * @param       resource
 *              Interrupt resource, portable type variable which is holding a
 *              previously saved interrupt context state.
 */
PORT_C_INLINE
void nsys_lock_exit(
    struct nsys_lock *          lock)
{
    nisr_set_mask(lock->isr_ctx);
}

/*--------------------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of sys_lock.h
 ******************************************************************************/
#endif /* SYS_LOCK_H */
