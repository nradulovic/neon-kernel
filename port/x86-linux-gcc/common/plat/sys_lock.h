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
 * @addtogroup  x86-linux-gcc
 *********************************************************************//** @{ */
/**@defgroup    x86-linux-gcc-critical Critical code section
 * @brief       Critical code section
 * @{ *//*--------------------------------------------------------------------*/

#ifndef SYS_LOCK_H
#define SYS_LOCK_H

/*=========================================================  INCLUDE FILES  ==*/

#include <pthread.h>
#include "plat/compiler.h"

/*===============================================================  MACRO's  ==*/
/*------------------------------------------------------  C++ extern begin  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/

struct nsys_lock
{
    uint8_t                     dummy;
}

typedef struct nsys_lock nsys_lock;

/*======================================================  GLOBAL VARIABLES  ==*/
/*===================================================  FUNCTION PROTOTYPES  ==*/


PORT_C_INLINE
void nsys_lock_init(void)
{
    extern pthread_mutex_t g_sys_lock;
    
    pthread_mutex_create(&g_sys_lock);
}



PORT_C_INLINE
void nsys_lock_term(void)
{
    extern pthread_mutex_t g_sys_lock;
    
    pthread_mutex_destroy(&g_sys_lock);
}



/**@brief       Enter critical code section
 * @param       lock
 *              Interrupt resource, pointer to portable type variable which will
 *              hold the interrupt context state during the critical code
 *              section.
 */
PORT_C_INLINE
void nsys_lock_enter(
    struct nsys_lock *          lock)
{
    extern pthread_mutex_t g_sys_lock;
    
    (void)lock;
    pthread_mutex_lock(&g_sys_lock);
}



/**@brief       Exit critical code section
 * @param       lock
 *              Interrupt resource, portable type variable which is holding a
 *              previously saved interrupt context state.
 */
PORT_C_INLINE
void nsys_lock_exit(
    struct nsys_lock *          lock)
{
    extern pthread_mutex_t g_sys_lock;
    
    (void)lock;
    pthread_mutex_unlock(&g_sys_lock);
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
