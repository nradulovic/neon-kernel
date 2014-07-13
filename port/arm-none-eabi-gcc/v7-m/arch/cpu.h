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
 * @brief       Interface of ARM Cortex CPU module port.
 * @addtogroup  arm-none-eabi-gcc
 *************************************************************************************************************//** @{ */
/**@defgroup    arm-none-eabi-gcc-v7-m-cpu ARM Cortex M3/M4 CPU module
 * @brief       CPU module
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

#ifndef ES_CPU_H_
#define ES_CPU_H_

/*=================================================================================================  INCLUDE FILES  ==*/

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "plat/compiler.h"
#include "arch/cortex_m3.h"

#include "lib/natomic.h"

/*=======================================================================================================  MACRO's  ==*/

/*----------------------------------------------------------------------------------------------------------------*//**
 * @name        Port constants
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

/**@brief       Specifies bit-width of general purpose registers
 */
#define NCPU_DATA_WIDTH                     32u

/**@brief       Specifies data alignment for optimal performance
 */
#define NCPU_DATA_ALIGNMENT                 4u

/**@} *//*--------------------------------------------------------------------------------------------------------*//**
 * @name        Bit operations
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

/**@brief       Find Last Set bit in a word
 */
#define NCPU_FIND_LAST_SET(val)             ncpu_find_last_set(val)

/**@brief       Compute exponential function with the base 2, f(x) = 2 ^ x
 */
#define NCPU_EXP2(pwr)                      ((uint32_t)0x01u << (pwr))

/**@} *//*--------------------------------------------------------------------------------------------------------*//**
 * @name        Generic port macros
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

#define NCPU_MODULE_INIT()                  (void)0

#define NCPU_MODULE_TERM()                  (void)0

#define NCPU_INIT()                         ncpu_init()

#define NCPU_INIT_LATE()                    (void)0

#define NCPU_TERM()                         (void)0

/**@} *//*--------------------------------------------------------------------------------------------------------*//**
 * @name        CPU register descriptions
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

/**@brief       PSR Thumb state: Position.
 */
#define PORT_PSR_THUMB_STATE_Pos            (24u)

/**@brief       PSR Thumb state: Mask.
 */
#define PORT_PSR_THUMB_STATE_Msk            (0x01ul << PORT_PSR_THUMB_STATE_Pos)

/*----------------------------------------------------------------------------------------------------------------*//**
 * @name        Port constants
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

/**@brief       Minimal stack size value is the number of elements in struct @ref esThreadCtx
 */
#define NCPU_STACK_MINSIZE                  (sizeof(struct nthread_ctx))

#define PORT_STACK_SIZE(size)               (size + NCPU_STACK_MINSIZE)

/**@} *//*--------------------------------------------------------------------------------------------------------*//**
 * @name        System timer constants
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

/**@brief       Threshold system timer value for new tick
 */
#define PORT_DEF_SYSTMR_WAKEUP_TH_VAL       600u

/**@} *//*--------------------------------------------------------------------------------------------------------*//**
 * @name        Kernel threads port dependent settings
 * @brief       Kernel uses several threads for system management. This section defines port dependent settings for the
 *              threads.
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

/**@brief       Kernel Virtual Timer Thread stack size
 * @todo        This value needs tweaking
 */
#define PORT_DEF_KVTMR_STCK_SIZE            40u

/**@brief       Kernel Idle Thread stack size
 * @todo        This value needs tweaking
 */
#define PORT_DEF_KIDLE_STCK_SIZE            40u

/**@} *//*--------------------------------------------------------------------------------------------------------*//**
 * @name        Interrupt management
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

#define NCPU_ISR_ENTER()                    (void)0

#define NCPU_ISR_EXIT()                     (void)0

#define NCPU_IS_ISR_LAST()                  ncpu_is_isr_last()

/**@} *//*--------------------------------------------------------------------------------------------------------*//**
 * @name        Dispatcher context switching support
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

#define NCPU_INIT_CTX(stack, stack_size, entry, arg)                                 \
    ncpu_init_ctx(stack, stack_size, entry, arg)

#define NCPU_DISPATCH()                     ncpu_dispatch()

#define NCPU_DISPATCH_FROM_ISR()            ncpu_dispatch()

#define NCPU_DISPATCH_TO_FIRST()            ncpu_dispatch_to_first()





/**@} *//*---------------------------------------------------------------------------------------  C++ extern base  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/

/**@brief       General purpose registers are 32bit wide.
 */
typedef unsigned int ncpu_reg;

/**@brief       Stack structure used for stack in order to force the alignment
 */
struct PORT_C_ALIGN(8) nthread_stack
{
    ncpu_reg            reg;
};

/**@brief       Stack type
 */
typedef struct nthread_stack nthread_stack;

/**@brief       Structure of the context switch
 * @details     There are 16, 32-bit wide core (integer) registers visible to the ARM and Thumb instruction sets.
 */
struct nthread_ctx
{
/* Registers saved by the context switcher                                    */
    ncpu_reg            r4;                                                     /**< @brief R4, Variable register 1   */
    ncpu_reg            r5;                                                     /**< @brief R5, Variable register 2   */
    ncpu_reg            r6;                                                     /**< @brief R6, Variable register 3   */
    ncpu_reg            r7;                                                     /**< @brief R7, Variable register 4   */
    ncpu_reg            r8;                                                     /**< @brief R8, Variable register 5   */
    ncpu_reg            r9;                                                     /**< @brief R9, Platform register     */
    ncpu_reg            r10;                                                    /**< @brief R10, Variable register 7  */
    ncpu_reg            r11;                                                    /**< @brief R11, Variable register 8  */

/* Registers saved by the hardware                                            */
    ncpu_reg            r0;                                                     /**< @brief R0, Argument/result       */
    ncpu_reg            r1;                                                     /**< @brief R1, Argument/result       */
    ncpu_reg            r2;                                                     /**< @brief R2, Argument/scratch      */
    ncpu_reg            r3;                                                     /**< @brief R3, Argument/scratch      */
    ncpu_reg            r12;                                                    /**< @brief IP, Intra-Procedure-call  */
    ncpu_reg            lr;                                                     /**< @brief LR, The Link Register     */
    ncpu_reg            pc;                                                     /**< @brief PC, The Program Counter   */
    ncpu_reg            xpsr;                                                   /**< @brief Special, Program Status   */
};

/*==============================================================================================  GLOBAL VARIABLES  ==*/
/*===========================================================================================  FUNCTION PROTOTYPES  ==*/

/*----------------------------------------------------------------------------------------------------------------*//**
 * @name        Bit operations
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

/**
 * @brief       Find last set bit in a word
 * @param       value
 *              32 bit value which will be evaluated
 * @return      Last set bit in a word
 * @details     This implementation uses @c clz instruction and then it computes
 *              the result using the following expression:
 *              <code>fls(x) = w − clz(x)</code>.
 * @inline
 */
static PORT_C_INLINE_ALWAYS uint_fast8_t ncpu_find_last_set(
    n_native                     value)
{
    uint_fast8_t                clz;

    __asm__ __volatile__ (
        "@  ncpu_find_last_set                              \n"
        "   clz    %0, %1                                   \n"
        : "=r"(clz)
        : "r"(value));

    return (31u - clz);
}

/**@} *//*--------------------------------------------------------------------------------------------------------*//**
 * @name        Generic port functions
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

static PORT_C_INLINE void ncpu_init(
    void)
{
    __asm__ __volatile__(
        "@  ncpu_init                                       \n"
        "   clrex                                           \n");               /* Clear the exclusive monitor.       */
}



/**@brief       Check if this is the last ISR executing
 * @return      Is the currently executed ISR the last one?
 *  @retval     true - this is last ISR
 *  @retval     false - this is not the last ISR
 * @inline
 */
static PORT_C_INLINE_ALWAYS bool ncpu_is_isr_last(
    void)
{
    if ((PORT_SCB->ICSR & PORT_SCB_ICSR_RETTOBASE_Msk) != 0u)  {
        return (true);
    } else {
        return (false);
    }
}

/**@} *//*--------------------------------------------------------------------------------------------------------*//**
 * @name        Dispatcher context switching
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

/**@brief       Initializes the thread context
 * @param       stack
 *              Pointer to the allocated thread stack. The pointer points to the
 *              beginning of the memory as defined per C language. The function
 *              will adjust the pointer according to the stack full descending
 *              type.
 * @param       stack_size
 *              The size of allocated stack in bytes.
 * @param       entry
 *              Pointer to the thread entry function.
 * @param       arg
 *              Argument that will be passed to thread function at the starting
 *              of execution.
 * @note        1) Interrupts are enabled when your task starts executing.
 * @note        2) All tasks run in Thread mode, using process stack.
 * @note        3) ARM Cortex M3 requires 8B aligned stack.
 */
void * ncpu_init_ctx(
    void *                      stack,
    size_t                      stack_size,
    void                     (* entry)(void *),
    void *                      arg);

/**@brief       Do the context switch
 * @details     This function will just initiate PendSV exception which will do
 *              the actual context switch
 * @inline
 */
static PORT_C_INLINE_ALWAYS void ncpu_dispatch(
    void)
{
    PORT_SCB->ICSR |= PORT_SCB_ICSR_PENDSVSET_Msk;
}

/**@brief       Start the first context switch
 * @details     This function will set the main stack register to point at the
 *              beginning of stack disregarding all previous stack information
 *              after which it will call system service to start the first
 *              thread.
 * @warning     This function requires valid Vector Table Offset Register in
 *              System control block. Vector Table Offset Register is used to
 *              extract the beginning of main stack.
 */
PORT_C_NORETURN void ncpu_dispatch_to_first(
    void);

/**@} *//*--------------------------------------------------------------------------------------------------------*//**
 * @name        Port specific functions
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

/**@brief       Pop the first thread stack
 * @details     During the thread initialization a false stack was created
 *              mimicking the real interrupt stack described in @ref esThreadCtx.
 *              With this function we restore the false stack and start the
 *              thread. This function is invoked only once from esKernStart()
 *              function.
 */
PORT_C_NAKED void port_svc_isr(
    void);

/**@brief       Execute context switching
 * @details     PendSV is used to cause a context switch. This is a recommended
 *              method for performing context switches with Cortex-M3. This is
 *              because the Cortex-M3 auto-saves half of the processor context
 *              on any exception, and restores same on return from exception. So
 *              only saving of R4-R11 is required and setting up the stack
 *              pointers. Using the PendSV exception this way means that context
 *              saving and restoring is identical whether it is initiated from a
 *              thread or occurs due to an interrupt or exception.
 */
PORT_C_NAKED void port_pend_sv(
    void);

/**@brief       System timer event handler
 * @details     System timer events are used by the scheduler
 */
void port_sys_tmr(
    void);


/**@} *//*--------------------------------------------------------------------------------------------------------*//**
 * @name        Atomic operations
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

static PORT_C_INLINE uint32_t ncpu_add(uint32_t number, struct natomic * atomic) {
    uint32_t                    result;
    ncpu_reg                    tmp;

    __asm__ __volatile__ (
        "@  ncpu_add                                        \n"
        "1: ldrex   %0, [%3]                                \n"
        "   add     %0, %0, %4                              \n"
        "   strex   %1, %0, [%3]                            \n"
        "   cmp     %1, #0                                  \n"
        "   bne     1b"
        : "=&r" (result), "=&r" (tmp), "+Qo" (atomic->counter)
        : "r" (&atomic->counter), "Ir" (number)
        : "cc");

    return (result);
}


/** @} *//*---------------------------------------------------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*========================================================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//**********************************************************************************************
 * END of cpu.h
 **********************************************************************************************************************/
#endif /* ES_CPU_H_ */
