/*
 * This file is part of NUB Real-Time Kernel.
 *
 * Copyright (C) 2010 - 2014 Nenad Radulovic
 *
 * NUB Real-Time Kernel is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * NUB Real-Time Kernel is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with NUB Real-Time Kernel.  If not, see <http://www.gnu.org/licenses/>.
 *
 * web site:    http://github.com/nradulovic
 * e-mail  :    nenad.b.radulovic@gmail.com
 *//***************************************************************************************************************//**
 * @file
 * @author  	Nenad Radulovic
 * @brief       Interface of ARM Cortex critical code section port.
 * @addtogroup  arm-none-eabi-gcc
 *********************************************************************//** @{ */
/**@defgroup    arm-none-eabi-gcc-critical Critical code section
 * @brief       Critical code section
 * @{ *//*--------------------------------------------------------------------*/

#ifndef ES_ARCH_CRITICAL_H_
#define ES_ARCH_CRITICAL_H_

/*=========================================================  INCLUDE FILES  ==*/

#include "plat/compiler.h"
#include "arch/intr.h"

/*===============================================================  MACRO's  ==*/

/*------------------------------------------------------------------------*//**
 * @name        Critical code lock management
 * @brief       Disable/enable interrupts by preserving the interrupt context
 * @details     Generally speaking these macros would store the interrupt
 *              context in the local variable of @ref esLockCtx type and then
 *              disable interrupts. Local variable is allocated in all of NUB RT Kernel
 *              functions that need to disable interrupts. Macros would restore
 *              the interrupt context by copying back the allocated variable
 *              into the interrupt controller status/control register.
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Enter critical code section
 * @param       lockCtx
 *              Interrupt context, pointer to portable type variable which will
 *              hold the interrupt context state during the critical code
 *              section.
 */
#define NCRITICAL_LOCK_ENTER(lockCtx)                                         \
    *(lockCtx) = nintr_replace_mask(NINTR_PRIO_TO_CODE(CONFIG_INTR_MAX_PRIO))

/**@brief       Exit critical code section
 * @param       lockCtx
 *              Interrupt context, portable type variable which is holding a
 *              previously saved interrupt context state.
 */
#define NCRITICAL_LOCK_EXIT(lockCtx)                                          \
    nintr_set_mask(lockCtx)

/**@} *//*----------------------------------------------  C++ extern begin  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/
/*======================================================  GLOBAL VARIABLES  ==*/
/*===================================================  FUNCTION PROTOTYPES  ==*/
/*--------------------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of critical.h
 ******************************************************************************/
#endif /* ES_ARCH_CRITICAL_H_ */
