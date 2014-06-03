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
 * @brief       Interrupt module header
 * @addtogroup  arm-none-eabi-gcc
 *********************************************************************//** @{ */
/**@defgroup    arm-none-eabi-gcc-v7-m-intr ARM Cortex M3/M4 Interrupt module
 * @brief       Interrupt module
 * @{ *//*--------------------------------------------------------------------*/

#ifndef ES_INTR_H_
#define ES_INTR_H_

/*=========================================================  INCLUDE FILES  ==*/

#include "plat/compiler.h"
#include "family/profile.h"
#include "cortex_m3.h"
#include "arch/intr_config.h"

/*===============================================================  MACRO's  ==*/

/*------------------------------------------------------------------------*//**
 * @name        Interrupt management
 * @{ *//*--------------------------------------------------------------------*/

#define ES_INTR_ENABLE()                portIntrEnable_()

#define ES_INTR_DISABLE()               portIntrDisable_()

#define ES_INTR_MASK_SET(mask)          portIntrMaskSet_(mask)

#define ES_INTR_MASK_GET(mask)          portIntrMaskGet_(mask)

#define ES_INTR_MASK_REPLACE(oldPrio, newPrio)                                  \
    portIntrMaskReplace_(oldPrio, newPrio)

#define ES_INTR_PRIO_TO_CODE(prio)                                              \
    (((prio) << (8u - PORT_ISR_PRIO_BITS)) & 0xfful)

#define ES_INTR_CODE_TO_PRIO(code)                                              \
    (((code) & 0xfful) >> (8u - PORT_ISR_PRIO_BITS))

#define ES_INTR_PRIO_SET(intrNum, prio) intrPrioSet_(intrNum, prio)

#define ES_INTR_PRIO_GET(intrNum, prio) intrPrioGet_(intrNum, prio)

/**@} *//*----------------------------------------------------------------*//**
 * @name        Generic port macros
 * @{ *//*--------------------------------------------------------------------*/

#define ES_MODULE_INTR_INIT()           portModuleIntrInit()

#define ES_MODULE_INTR_TERM()           portModuleIntrTerm()

#define ES_INTR_INIT()                  (void)0

#define ES_INTR_INIT_LATE()             (void)0                                 /**< @brief This port does not need this function call      */

/**@} *//*----------------------------------------------  C++ extern base  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/

/**@brief       Interrupt Number Definition
 * @details     Cortex-M3 Processor Exceptions Numbers
 */
enum esIntrN {
    NONMASKABLEINT_IRQN   = -14,                                                /**< @brief 2 Non Maskable Interrupt                        */
    MEMORYMANAGEMENT_IRQN = -12,                                                /**< @brief 4 Cortex-M3 Memory Management Interrupt         */
    BUSFAULT_IRQN         = -11,                                                /**< @brief 5 Cortex-M3 Bus Fault Interrupt                 */
    USAGEFAULT_IRQN       = -10,                                                /**< @brief 6 Cortex-M3 Usage Fault Interrupt               */
    SVCALL_IRQN           = -5,                                                 /**< @brief 11 Cortex-M3 SV Call Interrupt                  */
    PENDSV_IRQN           = -2,                                                 /**< @brief 14 Cortex-M3 Pend SV Interrupt                  */
    ES_SYSTEM_IRQN             = -1                                             /**< @brief 15 Cortex-M3 System Tick Interrupt              */
};

/**@brief       Interrupt context type
 * @details     This type is used to declare variable type which will hold
 *              interrupt context data.
 */
typedef unsigned int nintr_ctx;

/*======================================================  GLOBAL VARIABLES  ==*/
/*===================================================  FUNCTION PROTOTYPES  ==*/

/*------------------------------------------------------------------------*//**
 * @name        Interrupt management
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Enable all interrupts
 * @inline
 */
static PORT_C_INLINE_ALWAYS void portIntrEnable_(
    void) {

    __asm __volatile__ (
        "   cpsie   i                                       \n");
}

/**@brief       Disable all interrupts
 * @inline
 */
static PORT_C_INLINE_ALWAYS void portIntrDisable_(
    void) {

    __asm __volatile__ (
        "   cpsid   i                                       \n");
}

/**@brief       Set the new interrupt priority state
 * @param       state
 *              New interrupt priority mask or new state of interrupts
 * @note        Depending on @ref CONFIG_INTR_MAX_ISR_PRIO setting this function will
 *              either set the new priority of allowed interrupts or just
 *              disable/enable all interrupts.
 * @inline
 */
static PORT_C_INLINE_ALWAYS void portIntrMaskSet_(
    nintr_ctx           intrCtx) {

#if (0 != CONFIG_INTR_MAX_ISR_PRIO)
    __asm __volatile__ (
        "   msr    basepri, %0                              \n"
        :
        : "r"(intrCtx));
#else
    __asm __volatile__ (
        "   msr    primask, %0                              \n"
        :
        : "r"(intrCtx));
#endif
}

/**@brief       Get the interrupt priority state
 * @param       state
 *              Pointer to state variable where to store enabled interrupts
 * @inline
 */
static PORT_C_INLINE_ALWAYS void portIntrMaskGet_(
    nintr_ctx *         intrCtx) {

    nintr_ctx           tmp;

#if (0 != CONFIG_INTR_MAX_ISR_PRIO)
    __asm __volatile__ (
        "   mrs     %0, basepri                             \n"
        : "=r"(tmp));
#else
    __asm __volatile__ (
        "   mrs     %0, primask                             \n"
        : "=r"(tmp));
#endif
    *intrCtx = tmp;
}

/**@brief       Get old and set new interrupt priority mask
 * @return      Current interrupt priority mask
 * @inline
 */
static PORT_C_INLINE_ALWAYS void portIntrMaskReplace_(
    nintr_ctx *         old,
    nintr_ctx           new) {

    nintr_ctx           tmp;

#if (0 != CONFIG_INTR_MAX_ISR_PRIO)
    __asm __volatile__ (
        "   mrs     %0, basepri                             \n"
        "   msr     basepri, %1                             \n"
        : "=r"(tmp)
        : "r"(new));
#else
    __asm __volatile__ (
        "   mrs     %0, primask                             \n"
        "   msr    primask, %1                              \n"
        : "=r"(tmp)
        : "r"(new));
#endif
    *old = tmp;
}

/**@brief       Set Priority for Cortex-M  System Interrupts
 * @param       intrNum
 *              Interrupt number
 * @param       priority
 *              The priority of specified interrupt source. The parameter
 *              priority must be encoded with @ref ES_INTR_PRIO_TO_CODE.
 * @inline
 */
static PORT_C_INLINE_ALWAYS void intrPrioSet_(
    enum esIntrN        intrN,
    uint32_t            priority) {

    PORT_SCB->SHP[((uint32_t)(intrN) & 0x0ful) - 4u] = (uint8_t)priority;
}

/**@brief       Get Priority for Cortex-M  System Interrupts
 * @param       intrNum
 *              Interrupt number
 * @param       priority
 *              The priority of specified interrupt source. The parameter
 *              priority must be decoded with @ref ES_INTR_CODE_TO_PRIO.
 * @inline
 */
static PORT_C_INLINE_ALWAYS void intrPrioGet_(
    enum esIntrN        intrN,
    uint32_t *          priority) {

    *priority = PORT_SCB->SHP[((uint32_t)(intrN) & 0x0ful) - 4u];
}

/** @} *//*---------------------------------------------------------------*//**
 * @name        Generic port functions
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Initialize port
 * @details     Function will set up sub-priority bits to zero and handlers
 *              interrupt priority.
 */
void portModuleIntrInit(
    void);

/**@brief       Terminate port
 */
void portModuleIntrTerm(
    void);

/** @} *//*-----------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//** @} *//*********************************************
 * END of intr.h
 ******************************************************************************/
#endif /* ES_INTR_H_ */
