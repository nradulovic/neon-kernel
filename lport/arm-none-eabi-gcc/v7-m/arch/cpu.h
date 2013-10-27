/*
 * This file is part of eSolid
 *
 * Copyright (C) 2011, 2012, 2013 - Nenad Radulovic
 *
 * eSolid is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * eSolid is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * eSolid; if not, write to the Free Software Foundation, Inc., 51 Franklin St,
 * Fifth Floor, Boston, MA  02110-1301  USA
 *
 * web site:    http://github.com/nradulovic
 * e-mail  :    nenad.b.radulovic@gmail.com
 *//***********************************************************************//**
 * @file
 * @author  	Nenad Radulovic
 * @brief       Interface of ARM Cortex-M3 CPU port.
 * @addtogroup  arm-none-eabi-gcc-v7-m
 *********************************************************************//** @{ */

#if !defined(CPU_H_)
#define CPU_H_

/*=========================================================  INCLUDE FILES  ==*/

#include "arch/compiler.h"
#include "arch/core.h"
#include "arch/cpu_cfg.h"
#include "kernel/kernel_cfg.h"

/*===============================================================  MACRO's  ==*/

/*------------------------------------------------------------------------*//**
 * @name        Port constants
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Minimal stack size value is the number of elements in struct
 *              @ref lpCtx
 */
#define LP_DEF_STCK_MINSIZE                                                  \
    (sizeof(struct lpCtx) / sizeof(portReg_T))

/**@brief       System timer one tick value
 */
#define LP_DEF_SYSTMR_ONE_TICK                                               \
    (CFG_SYSTMR_CLOCK_FREQUENCY / CFG_SYSTMR_EVENT_FREQUENCY)

/**@brief       Maximum number of ticks the system timer can accept
 */
#define LP_DEF_SYSTMR_MAX_TICKS                                              \
    (PORT_SYSTMR_MAX_VAL / LP_DEF_SYSTMR_ONE_TICK)

/**@brief       Kernel Virtual Timer Thread stack size
 */
#define LP_KVTMR_THD_STCK_SIZE           40U

/**@brief       Kernel Idle Thread stack size
 */
#define LP_KIDLE_THD_STCK_SIZE           40U

/** @} *//*---------------------------------------------------------------*//**
 * @name        Dispatcher context switching
 * @{ *//*--------------------------------------------------------------------*/

#define LP_CTX_INIT(stck, stckSize, thread, arg)                              \
    lpCtxInit(stck, stckSize, thread, arg)

#define LP_CTX_SW()                   lpCtxSw_()

/**@brief       This port has identical context switch functions.
 */
#define LP_CTX_SW_ISR()               LP_CTX_SW()

#define LP_THD_START()                lpThdStart()

/** @} *//*---------------------------------------------------------------*//**
 * @name        Generic port macros
 * @{ *//*--------------------------------------------------------------------*/

#define LP_STCK_SIZE(size)                                                    \
    ((((size + LP_DEF_STCK_MINSIZE) + (sizeof(struct lpStck) /              \
    sizeof(portReg_T))) - 1U) / (sizeof(struct lpStck)/sizeof(portReg_T)))

/** @} *//*---------------------------------------------------------------*//**
 * @name        Port specific macros
 * @{ *//*--------------------------------------------------------------------*/


/** @} *//*---------------------------------------------  C++ extern begin  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/

/**@brief       Stack structure used for stack in order to force the alignment
 */
PORT_C_ALIGNED(8) struct lpStck {
    portReg_T       reg;
};

/**@brief       Stack type
 */
typedef struct lpStck lpStck_T;

/**@brief       Structure of the context switch
 * @details     There are 16, 32-bit core (integer) registers visible to the ARM
 *              and Thumb instruction sets.
 */
struct lpCtx {
/* Registers saved by the context switcher                                    */
    portReg_T         r4;                                                       /**< @brief R4, Variable register 1                         */
    portReg_T         r5;                                                       /**< @brief R5, Variable register 2                         */
    portReg_T         r6;                                                       /**< @brief R6, Variable register 3                         */
    portReg_T         r7;                                                       /**< @brief R7, Variable register 4                         */
    portReg_T         r8;                                                       /**< @brief R8, Variable register 5                         */
    portReg_T         r9;                                                       /**< @brief R9, Platform register/variable register 6       */
    portReg_T         r10;                                                      /**< @brief R10, Variable register 7                        */
    portReg_T         r11;                                                      /**< @brief R11, Variable register 8                        */

/* Registers saved by the hardware                                            */
    portReg_T         r0;                                                       /**< @brief R0, Argument/result/scratch register 1          */
    portReg_T         r1;                                                       /**< @brief R1, Argument/result/scratch register 2          */
    portReg_T         r2;                                                       /**< @brief R2, Argument/scratch register 3                 */
    portReg_T         r3;                                                       /**< @brief R3, Argument/scratch register 3                 */
    portReg_T         r12;                                                      /**< @brief R12, IP, The Intra-Procedure-call scratch reg.  */
    portReg_T         lr;                                                       /**< @brief R14, LR, The Link Register                      */
    portReg_T         pc;                                                       /**< @brief R15, PC, The Program Counter                    */
    portReg_T         xpsr;                                                     /**< @brief Special, Program Status Register                */
};

/*======================================================  GLOBAL VARIABLES  ==*/
/*===================================================  FUNCTION PROTOTYPES  ==*/

/*------------------------------------------------------------------------*//**
 * @name        Scheduler support
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Start the first thread
 * @details     This function will set the main stack register to point at the
 *              beginning of stack disregarding all previous stack information
 *              after which it will call system service to start the first
 *              thread.
 * @warning     This function requires valid Vector Table Offset Register in
 *              System control block. Vector Table Offset Register is used to
 *              extract the beginning of main stack.
 */
PORT_C_NORETURN void lpThdStart(
    void);

/** @} *//*---------------------------------------------------------------*//**
 * @name        Dispatcher context switching
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Do the context switch
 * @details     This function will just initiate PendSV exception which will do
 *              the actual context switch
 * @inline
 */
PORT_C_INLINE_ALWAYS static void lpCtxSw_(
    void) {

    *CPU_SCB_ICSR |= CPU_SCB_ICSR_PENDSVSET_MSK;
}

/**@brief       Initializes the thread context
 * @param       stck
 *              Pointer to the allocated thread stack. The pointer points to the
 *              beginning of the memory as defined per C language. The function
 *              will adjust the pointer according to the stack full descending
 *              type.
 * @param       stckSize
 *              The size of allocated stack in bytes.
 * @param       fn
 *              Pointer to the thread function.
 * @param       arg
 *              Argument that will be passed to thread function at the starting
 *              of execution.
 * @note        1) Interrupts are enabled when your task starts executing.
 * @note        2) All tasks run in Thread mode, using process stack.
 * @note        3) ARM Cortex M3 requires 8B aligned stack.
 */
void * lpCtxInit(
    void *          stck,
    size_t          stckSize,
    void (* fn)(void *),
    void *          arg);

/** @} *//*---------------------------------------------------------------*//**
 * @name        Generic port functions
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Initialize port
 * @details     Function will set up sub-priority bits to zero and handlers
 *              interrupt priority.
 */
void lpInitEarly(
    void);

/** @} *//*---------------------------------------------------------------*//**
 * @name        Port specific functions
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Pop the first thread stack
 * @details     During the thread initialization a false stack was created
 *              mimicking the real interrupt stack described in @ref lpCtx.
 *              With this function we restore the false stack and start the
 *              thread. This function is invoked only once from esKernStart()
 *              function.
 */
PORT_C_NAKED void portSVC(
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
PORT_C_NAKED void portPendSV(
    void);

/**@brief       System timer event handler
 * @details     System timer events are used by the scheduler
 */
void portSysTmr(
    void);

/** @} *//*-----------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/

#if (PORT_DEF_SYSTMR_MAX_VAL < LP_DEF_SYSTMR_ONE_TICK)
# error "eSolid RT Kernel port: System Timer overflow, please check CFG_SYSTMR_CLOCK_FREQUENCY and CFG_SYSTMR_EVENT_FREQUENCY options."
#endif

/** @endcond *//** @} *//******************************************************
 * END of cpu.h
 ******************************************************************************/
#endif /* CPU_H_ */
