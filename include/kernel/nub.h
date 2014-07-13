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
 *//***********************************************************************//**
 * @file
 * @author  	Nenad Radulovic
 * @brief       Main kernel interface
 * @defgroup    kernel Kernel
 * @brief       Kernel overview
 * @details     Main kernel interface is divided into several sections.
 *********************************************************************//** @{ */
/**@defgroup    kernel_intf Interface
 * @brief       Kernel main interface
 * @{ *//*--------------------------------------------------------------------*/

#ifndef NUB_H_
#define NUB_H_

/*=========================================================  INCLUDE FILES  ==*/

#include "plat/compiler.h"
#include "plat/critical.h"

/*===============================================================  MACRO's  ==*/

/*------------------------------------------------------------------------*//**
 * @defgroup    kernel_generic Kernel generic services and identification
 * @brief       Kernel generic services and unique identification
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Identifies kernel major version number
 */
#define NSYS_VER_MAJOR                      (1ul)

/**@brief       Identifies kernel minor version number
 */
#define NSYS_VER_MINOR                      (0ul)

/**@brief       Identifies kernel patch level
 */
#define NSYS_VER_PATCH                      (0ul)

/**@brief       Identifies the underlying kernel version number
 */
#define NSYS_VER                            (((NSYS_VER_MAJOR) << 24) | (NSYS_VER_MINOR << 16) | (NSYS_VER_PATCH))

/**@brief       Kernel identification string
 */
#define NSYS_ID                             "NUB Real-Time Kernel"

/*------------------------------------------------------  C++ extern begin  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/
/*======================================================  GLOBAL VARIABLES  ==*/
/*===================================================  FUNCTION PROTOTYPES  ==*/

/*------------------------------------------------------------------------*//**
 * @addtogroup  kernel_generic
 * @{ *//*--------------------------------------------------------------------*/


/**@brief       Initialize kernel internal data structures
 * @pre         1) `The kernel state == ES_KERN_INACTIVE`, see @ref states.
 * @post        1) `The kernel state == ES_KERN_INIT`.
 * @note        1) This function may be invoked only once.
 * @details     This function must be called first before any other kernel API. It initializes internal data structures
 *              that are used by other kernel functions.
 * @called
 * @fromapp
 * @schedno
 * @api
 */
void nsys_init(
    void);



/**@brief       Start the multi-threading
 * @pre         1) `The kernel state == ES_KERN_INIT`, see @ref states.
 * @post        1) `The kernel state == ES_KERN_RUN`
 * @post        2) The multi-threading execution will commence.
 * @note        1) Once this function is called the execution of threads will start and this function will never return.
 * @details     This function will start multi-threading. Once the multi-threading has started the execution will never
 *              return to this function again (this function never returns).
 * @called
 * @fromapp
 * @schedyes
 * @api
 */
PORT_C_NORETURN void nsys_start(
    void);



/**@brief       Enter Interrupt Service Routine
 * @pre         1) `The kernel state < ES_KERN_INIT`, see @ref states.
 * @note        1) You must call esKernIsrExitI() at the exit of ISR.
 * @note        2) You must invoke nsys_isr_enter_i() and nsys_isr_exit_i() in pair. In other words, for every call to
 *                  nsys_isr_enter_i() at the beginning of the ISR you must have a call to nsys_isr_exit_i() at the end
 *                  of the ISR.
 * @details     Function will notify kernel that you are about to enter interrupt service routine (ISR). This allows
 *              kernel to keep track of interrupt nesting and then only perform rescheduling at the last nested ISR.
 * @called
 * @fromisr
 * @schedno
 * @iclass
 */
void nsys_isr_enter_i(
    void);



/**@brief       Exit Interrupt Service Routine
 * @pre         1) `The kernel state < ES_KERN_INIT`, see @ref states.
 * @note        1) You must invoke nsys_isr_enter_i() and nsys_isr_exit_i() in pair. In other words, for every call to
 *                  nsys_isr_enter_i() at the beginning of the ISR you must have a call to nsys_isr_exit_i() at the end
 *                  of the ISR.
 * @note        2) Rescheduling is prevented when the kernel is locked (see esKernLockEnterI())
 * @details     This function is used to notify kernel that you have completed servicing an interrupt. When the last
 *              nested ISR has completed, the function will call the scheduler to determine whether a new, high-priority
 *              task, is ready to run.
 * @called
 * @fromisr
 * @schedmaybe
 * @iclass
 */
void nsys_isr_exit_i(
    void);



/**@brief       Process the timer event
 * @pre         1) `The kernel state < ES_KERN_INIT`, see @ref states.
 * @details     This function will be called only by port timer interrupt.
 * @notapi
 */
void nsys_timer_isr(
    void);



void nsys_lock_enter(
    void);



void nsys_lock_exit(
    void);



void nsys_lock_int_enter(
    nintr_ctx *                 intr_ctx);



void nsys_lock_int_exit(
    nintr_ctx                   intr_ctx);

/** @} *//*---------------------------------------------------------------*//**
 * @addtogroup  Hook functions
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Hook function called at system early init
 * @details     This function is called only when @ref CONFIG_HOOK_SYS_EARLY_INIT is enabled.
 * @api
 */
extern void hook_at_sys_early_init(
    void);



/**@brief       Hook function called at system late init
 * @details     This function is called only when @ref CONFIG_HOOK_AT_SYS_LATE_INIT is enabled.
 * @api
 */
extern void hook_at_sys_late_init(
    void);



/**@brief       Hook function called at system start
 * @details     This function is called only when @ref CONFIG_HOOK_AT_SYS_START is enabled.
 * @api
 */
extern void hook_at_sys_start(
    void);


/** @} *//*-----------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//**@} *//**@} *//***********************************************
 * END of nub.h
 ******************************************************************************/
#endif /* NUB_H_ */
