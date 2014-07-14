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
 *//***************************************************************************************************************//**
 * @file
 * @author  	Nenad Radulovic
 * @brief       Interrupt module header
 * @addtogroup  arm-none-eabi-gcc
 *************************************************************************************************************//** @{ */
/**@defgroup    arm-none-eabi-gcc-v7-m-intr ARM Cortex M3/M4 Interrupt module
 * @brief       Interrupt module
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

#ifndef NINTR_H_
#define NINTR_H_

/*=================================================================================================  INCLUDE FILES  ==*/

#include "plat/compiler.h"
#include "family/profile.h"
#include "cortex_m3.h"
#include "arch/intr_config.h"

/*=======================================================================================================  MACRO's  ==*/

/*----------------------------------------------------------------------------------------------------------------*//**
 * @name        Interrupt management
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

#define NINTR_ENABLE()                      nintr_enable()

#define NINTR_DISABLE()                     nintr_disable()

#define NINTR_SET_MASK(mask)                nintr_set_mask(mask)

#define NINTR_GET_MASK(mask)                nintr_get_mask(mask)

#define NINTR_REPLACE_MASK(newPrio)                                                                                     \
    nintr_replace_mask(newPrio)

#define NINTR_PRIO_TO_CODE(prio)                                                                                        \
    (((prio) << (8u - PORT_ISR_PRIO_BITS)) & 0xfful)

#define NINTR_CODE_TO_PRIO(code)                                                                                        \
    (((code) & 0xfful) >> (8u - PORT_ISR_PRIO_BITS))

#define NINTR_SET_PRIORITY(intr_num, prio)  nintr_set_priority(intr_num, prio)

#define NINTR_GET_PRIORITY(intr_num, prio)  nintr_get_priority(intr_num, prio)

/**@} *//*--------------------------------------------------------------------------------------------------------*//**
 * @name        Generic port macros
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

#define NINTR_MODULE_INIT()                 nintr_module_init()

#define NINTR_MODULE_TERM()                 nintr_module_term()

#define NINTR_INIT()                        (void)0

#define NINTR_INIT_LATE()                   (void)0                                 /**< @brief This port does not need this function call      */

/**@} *//*----------------------------------------------  C++ extern base  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*====================================================================================================  DATA TYPES  ==*/

/**@brief       Interrupt Number Definition
 * @details     Cortex-M3 Processor Exceptions Numbers
 */
enum nintr_no
{
    NONMASKABLEINT_IRQN   = -14,                                                /**< @brief 2 Non Maskable Interrupt                        */
    MEMORYMANAGEMENT_IRQN = -12,                                                /**< @brief 4 Cortex-M3 Memory Management Interrupt         */
    BUSFAULT_IRQN         = -11,                                                /**< @brief 5 Cortex-M3 Bus Fault Interrupt                 */
    USAGEFAULT_IRQN       = -10,                                                /**< @brief 6 Cortex-M3 Usage Fault Interrupt               */
    SVCALL_IRQN           = -5,                                                 /**< @brief 11 Cortex-M3 SV Call Interrupt                  */
    PENDSV_IRQN           = -2,                                                 /**< @brief 14 Cortex-M3 Pend SV Interrupt                  */
    ES_SYSTEM_IRQN        = -1                                             /**< @brief 15 Cortex-M3 System Tick Interrupt              */
};

/**@brief       Interrupt context type
 * @details     This type is used to declare variable type which will hold
 *              interrupt context data.
 */
typedef unsigned int nintr_ctx;

/*==============================================================================================  GLOBAL VARIABLES  ==*/
/*===========================================================================================  FUNCTION PROTOTYPES  ==*/

/*----------------------------------------------------------------------------------------------------------------*//**
 * @name        Interrupt management
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

/**@brief       Enable all interrupts
 * @inline
 */
static PORT_C_INLINE_ALWAYS void nintr_enable(
    void) {

    __asm __volatile__ (
        "@  nintr_enable                                    \n"
        "   cpsie   i                                       \n");
}

/**@brief       Disable all interrupts
 * @inline
 */
static PORT_C_INLINE_ALWAYS void nintr_disable(
    void) {

    __asm __volatile__ (
        "@  nintr_disable                                   \n"
        "   cpsid   i                                       \n");
}

/**@brief       Set the new interrupt priority state
 * @param       state
 *              New interrupt priority mask or new state of interrupts
 * @note        Depending on @ref CONFIG_INTR_MAX_PRIO setting this function will
 *              either set the new priority of allowed interrupts or just
 *              disable/enable all interrupts.
 * @inline
 */
static PORT_C_INLINE_ALWAYS void nintr_set_mask(
    nintr_ctx                   intrCtx) {

#if (0 != CONFIG_INTR_MAX_PRIO)
    __asm __volatile__ (
        "@  nintr_set_mask                                  \n"
        "   msr    basepri, %0                              \n"
        :
        : "r"(intrCtx));
#else
    __asm __volatile__ (
        "@  nintr_set_mask                                  \n"
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
static PORT_C_INLINE_ALWAYS void nintr_get_mask(
    nintr_ctx *                 intrCtx) {

    nintr_ctx                   tmp;

#if (0 != CONFIG_INTR_MAX_PRIO)
    __asm __volatile__ (
        "@  nintr_get_mask                                  \n"
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
static PORT_C_INLINE_ALWAYS nintr_ctx nintr_replace_mask(
    nintr_ctx                   new)
{
    nintr_ctx                   old;

#if (0 != CONFIG_INTR_MAX_PRIO)
    __asm __volatile__ (
        "@  nintr_replace_mask                              \n"
        "   mrs     %0, basepri                             \n"
        "   msr     basepri, %1                             \n"
        : "=&r"(old)
        : "r"(new));
#else
    __asm __volatile__ (
        "@  nintr_replace_mask                              \n"
        "   mrs     %0, primask                             \n"
        "   msr    primask, %1                              \n"
        : "=&r"(old)
        : "r"(new));
#endif

    return (old);
}

/**@brief       Set Priority for Cortex-M  System Interrupts
 * @param       intrNum
 *              Interrupt number
 * @param       priority
 *              The priority of specified interrupt source. The parameter priority must be encoded with
 *              @ref ES_INTR_PRIO_TO_CODE.
 * @inline
 */
static PORT_C_INLINE_ALWAYS void nintr_set_priority(
    enum nintr_no                intr_no,
    uint32_t                    priority) {

    PORT_SCB->SHP[((uint32_t)(intr_no) & 0x0ful) - 4u] = (uint8_t)priority;
}

/**@brief       Get Priority for Cortex-M  System Interrupts
 * @param       intrNum
 *              Interrupt number
 * @param       priority
 *              The priority of specified interrupt source. The parameter priority must be decoded with
 *              @ref NINTR_CODE_TO_PRIO.
 * @inline
 */
static PORT_C_INLINE_ALWAYS void nintr_get_priority(
    enum nintr_no                intr_no,
    uint32_t *                  priority) {

    *priority = PORT_SCB->SHP[((uint32_t)(intr_no) & 0x0ful) - 4u];
}

/** @} *//*-------------------------------------------------------------------------------------------------------*//**
 * @name        Generic port functions
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

/**@brief       Initialize port
 * @details     Function will set up sub-priority bits to zero and handlers
 *              interrupt priority.
 */
void nintr_module_init(
    void);

/**@brief       Terminate port
 */
void nintr_module_term(
    void);

/** @} *//*-----------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*========================================================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//** @} *//*************************************************************************************
 * END of intr.h
 **********************************************************************************************************************/
#endif /* NINTR_H_ */
