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
 * @brief       Interface of ARM Cortex-M3 CPU port.
 * @addtogroup  arm-none-eabi-gcc-v7-m
 *********************************************************************//** @{ */

#if !defined(CPU_H__)
#define CPU_H__

/*=========================================================  INCLUDE FILES  ==*/

#include "arch/compiler.h"
#include "arch/cpu_cfg.h"
#include "kernel_cfg.h"

/*===============================================================  MACRO's  ==*/

/*------------------------------------------------------------------------*//**
 * @name        Port constants
 * @{ *//*--------------------------------------------------------------------*/

#define PORT_DEF_DATA_WIDTH             32u

#define PORT_DEF_DATA_ALIGNMENT         4u

#define PORT_DEF_MAX_ISR_PRIO                                                       \
    (((PORT_CFG_MAX_ISR_PRIO) << (8u - CPU_DEF_ISR_PRIO_BITS)) & 0xfful)

/**@brief       Minimal stack size value is the number of elements in struct
 *              @ref portCtx
 */
#define PORT_DEF_STCK_MINSIZE                                                   \
    (sizeof(struct portCtx) / sizeof(portReg_T))

/**@} *//*----------------------------------------------------------------*//**
 * @name        System timer constants
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       System timer one tick value
 */
#define PORT_DEF_SYSTMR_ONE_TICK                                                \
    (PORT_CFG_SYSTMR_CLOCK_FREQ / CFG_SYSTMR_EVENT_FREQUENCY)

/**@brief       Maximum number of ticks without overflowing the system timer
 */
#define PORT_DEF_SYSTMR_MAX_TICKS                                               \
    (CPU_DEF_SYSTMR_MAX_VAL / PORT_DEF_SYSTMR_ONE_TICK)

/**@brief       Threshold system timer value for new tick
 */
#define PORT_DEF_SYSTMR_WAKEUP_TH_VAL   600u

/**@} *//*----------------------------------------------------------------*//**
 * @name        Kernel threads port dependent settings
 * @brief       Kernel uses several threads for system management. This section
 *              defines port dependent settings for the threads.
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Kernel Virtual Timer Thread stack size
 * @todo        This value needs tweaking
 */
#define PORT_DEF_KVTMR_STCK_SIZE        40u

/**@brief       Kernel Idle Thread stack size
 * @todo        This value needs tweaking
 */
#define PORT_DEF_KIDLE_STCK_SIZE        40u

/**@} *//*----------------------------------------------------------------*//**
 * @name        Interrupt management
 * @{ *//*--------------------------------------------------------------------*/

#define PORT_INT_ENABLE()               portIntEnable_()

#define PORT_INT_DISABLE()              portIntDisable_()

#define PORT_INT_PRIO_SET(prio)         portIntPrioSet_(prio)

#define PORT_INT_PRIO_GET(prio)         portIntPrioGet_(prio)

#define PORT_INT_PRIO_REPLACE(oldPrio, newPrio)                                 \
    portIntPrioReplace_(oldPrio, newPrio)

#define PORT_ISR_ENTER()                (void)0                                 /**< @brief This port does not need this function call      */

#define PORT_ISR_EXIT()                 (void)0                                 /**< @brief This port does not need this function call      */

#define PORT_ISR_IS_LAST()              portIsrIsLast_()

/**@} *//*----------------------------------------------------------------*//**
 * @name        System timer management
 * @{ *//*--------------------------------------------------------------------*/

#define PORT_SYSTMR_INIT(val)           portSysTmrInit_(val)

#define PORT_SYSTMR_TERM()              portSysTmrTerm_()

#define PORT_SYSTMR_GET_RVAL()          portSysTmrGetRVal_()

#define PORT_SYSTMR_GET_CVAL()          portSysTmrGetCVal_()

#define PORT_SYSTMR_RLD(val)            portSysTmrRld_(val)

#define PORT_SYSTMR_ENABLE()            portSysTmrEnable_()

#define PORT_SYSTMR_DISABLE()           portSysTmrDisable_()

#define PORT_SYSTMR_ISR_ENABLE()        portSysTmrIsrEnable_()

#define PORT_SYSTMR_ISR_DISABLE()       portSysTmrIsrDisable_()

/**@} *//*----------------------------------------------------------------*//**
 * @name        Scheduler bit operations support
 * @{ *//*--------------------------------------------------------------------*/

#define PORT_BIT_FIND_LAST_SET(val)     portBitFindLastSet_(val)

#define PORT_BIT_PWR2(pwr)              (0x01u << (pwr))

/**@} *//*----------------------------------------------------------------*//**
 * @name        Dispatcher context switching support
 * @{ *//*--------------------------------------------------------------------*/

#define PORT_CTX_INIT(stck, stckSize, thd, arg)                                 \
    portCtxInit(stck, stckSize, thd, arg)

#define PORT_CTX_SW()                   portCtxSw_()

#define PORT_CTX_SW_ISR()               portCtxSw_()

#define PORT_CTX_SW_START()             portCtxSwStart()

/** @} *//*---------------------------------------------------------------*//**
 * @name        Generic port macros
 * @{ *//*--------------------------------------------------------------------*/

#define PORT_CPU_INIT_EARLY()           (void)0                                 /**< @brief This port does not need this function call      */

#define PORT_CPU_INIT()                 portCpuInit()

#define PORT_CPU_INIT_LATE()            (void)0                                 /**< @brief This port does not need this function call      */

#define PORT_CPU_TERM()                 portCpuTerm()

#define PORT_STCK_SIZE(size)                                                    \
    (size + PORT_DEF_STCK_MINSIZE)

/** @} *//*---------------------------------------------------------------*//**
 * @name        Port specific macros
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       System Control Space Base Address.
 */
#define CPU_SCS_BASE                    (0xe000e000ul)

/**@brief       System Control Block Base Address.
 */
#define CPU_SCB_BASE                    (CPU_SCS_BASE + 0x0d00ul)

/**@brief       Interrupt Control and State Register Base Address Offset.
 */
#define CPU_SCB_ICSR_OFFSET             (0x04ul)

/**@brief       SCB Interrupt Control State Register
 */
#define CPU_SCB_ICSR                                                            \
    ((volatile portReg_T *)(CPU_SCB_BASE + CPU_SCB_ICSR_OFFSET))

/**@brief       SCB icsr: PENDSVSET Position.
 */
#define CPU_SCB_ICSR_PENDSVSET_POS      (28u)

/**@brief       SCB icsr: PENDSVSET Mask.
 */
#define CPU_SCB_ICSR_PENDSVSET_MSK      (0x01ul << CPU_SCB_ICSR_PENDSVSET_POS)

/**@brief       SCB icsr: PENDSTCLR Position.
 */
#define CPU_SCB_ICSR_PENDSTCLR_POS      (25u)

/**@brief       SCB icsr: PENDSTCLR Mask.
 */
#define CPU_SCB_ICSR_PENDSTCLR_MSK      (0x01ul << CPU_SCB_ICSR_PENDSTCLR_POS)

/**@brief       SCB icsr: RETTOBASE Position.
 */
#define CPU_SCB_ICSR_RETTOBASE_POS      (11u)

/**@brief       SCB icsr: RETTOBASE Mask.
 */
#define CPU_SCB_ICSR_RETTOBASE_MSK      (0x01ul << CPU_SCB_ICSR_RETTOBASE_POS)

/**@brief       System Timer Base Address.
 */
#define CPU_SYST_BASE                   (CPU_SCS_BASE + 0x0010ul)

/**@brief       Control and Status Register Base Address Offset.
 */
#define CPU_SYST_CSR_OFFSET             (0x0ul)

/**@brief       SysTick Control and Status Register
 */
#define CPU_SYST_CSR                                                            \
    ((volatile portReg_T *)(CPU_SYST_BASE + CPU_SYST_CSR_OFFSET))

/**@brief       SYSTMR csr: CLKSOURCE Position.
 */
#define CPU_SYST_CSR_CLKSOURCE_POS      (2u)

/**@brief       SYSTMR csr: CLKSOURCE Mask.
 */
#define CPU_SYST_CSR_CLKSOURCE_MSK      (0x01ul << CPU_SYST_CSR_CLKSOURCE_POS)

/**@brief       SYSTMR csr: TICKINT Position.
 */
#define CPU_SYST_CSR_TICKINT_POS        (1u)

/**@brief       SYSTMR csr: TICKINT Mask.
 */
#define CPU_SYST_CSR_TICKINT_MSK        (0x01ul << CPU_SYST_CSR_TICKINT_POS)

/**@brief       SYSTMR csr: ENABLE Position.
 */
#define CPU_SYST_CSR_ENABLE_POS         (0u)

/**@brief       SYSTMR csr: ENABLE Mask.
 */
#define CPU_SYST_CSR_ENABLE_MSK         (0x01ul << CPU_SYST_CSR_ENABLE_POS)

/**@brief       Control and Status Register Base Address Offset.
 */
#define CPU_SYST_RVR_OFFSET             (0x04ul)

/**@brief       SysTick Reload Value Register
 */
#define CPU_SYST_RVR                                                            \
    ((volatile portReg_T *)(CPU_SYST_BASE + CPU_SYST_RVR_OFFSET))

/**@brief       Control and Status Register Base Address Offset.
 */
#define CPU_SYST_CVR_OFFSET             (0x08ul)

/**@brief       SysTick Current Value Register
 */
#define CPU_SYST_CVR                                                            \
    ((volatile portReg_T *)(CPU_SYST_BASE + CPU_SYST_CVR_OFFSET))

/** @} *//*---------------------------------------------  C++ extern begin  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/

/**@brief General purpose registers are 32bit wide.
 */
typedef uint32_t portReg_T;

/**@brief       System timer hardware register type.
 */
typedef uint32_t portSysTmrReg_T;

/**@brief       Stack structure used for stack in order to force the alignment
 */
struct portStck {
    uint32_t            reg;
} __attribute__ ((aligned (8)));

/**@brief       Stack type
 */
typedef struct portStck portStck_T;

/**@brief       Structure of the context switch
 * @details     There are 16, 32-bit wide core (integer) registers visible to
 *              the ARM and Thumb instruction sets.
 */
struct portCtx {
/* Registers saved by the context switcher                                    */
    portReg_T           r4;                                                     /**< @brief R4, Variable register 1                         */
    portReg_T           r5;                                                     /**< @brief R5, Variable register 2                         */
    portReg_T           r6;                                                     /**< @brief R6, Variable register 3                         */
    portReg_T           r7;                                                     /**< @brief R7, Variable register 4                         */
    portReg_T           r8;                                                     /**< @brief R8, Variable register 5                         */
    portReg_T           r9;                                                     /**< @brief R9, Platform register/variable register 6       */
    portReg_T           r10;                                                    /**< @brief R10, Variable register 7                        */
    portReg_T           r11;                                                    /**< @brief R11, Variable register 8                        */

/* Registers saved by the hardware                                            */
    portReg_T           r0;                                                     /**< @brief R0, Argument/result/scratch register 1          */
    portReg_T           r1;                                                     /**< @brief R1, Argument/result/scratch register 2          */
    portReg_T           r2;                                                     /**< @brief R2, Argument/scratch register 3                 */
    portReg_T           r3;                                                     /**< @brief R3, Argument/scratch register 3                 */
    portReg_T           r12;                                                    /**< @brief R12, IP, The Intra-Procedure-call scratch reg.  */
    portReg_T           lr;                                                     /**< @brief R14, LR, The Link Register                      */
    portReg_T           pc;                                                     /**< @brief R15, PC, The Program Counter                    */
    portReg_T           xpsr;                                                   /**< @brief Special, Program Status Register                */
};

/*======================================================  GLOBAL VARIABLES  ==*/
/*===================================================  FUNCTION PROTOTYPES  ==*/

/*------------------------------------------------------------------------*//**
 * @name        Interrupt management
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Enable all interrupts
 * @inline
 */
static PORT_C_INLINE_ALWAYS void portIntEnable_(
    void) {

    __asm __volatile__ (
        "   cpsie   i                                       \n");
}

/**@brief       Disable all interrupts
 * @inline
 */
static PORT_C_INLINE_ALWAYS void portIntDisable_(
    void) {

    __asm __volatile__ (
        "   cpsid   i                                       \n");
}

/**@brief       Set the new interrupt priority state
 * @param       state
 *              New interrupt priority mask or new state of interrupts
 * @note        Depending on @ref PORT_CFG_MAX_ISR_PRIO setting this function will
 *              either set the new priority of allowed interrupts or just
 *              disable/enable all interrupts.
 * @inline
 */
static PORT_C_INLINE_ALWAYS void portIntPrioSet_(
    portReg_T           state) {

#if (0 != PORT_CFG_MAX_ISR_PRIO)
    __asm __volatile__ (
        "   msr    basepri, %0                              \n"
        :
        : "r"(state));
#else
    __asm __volatile__ (
        "   msr    primask, %0                              \n"
        :
        : "r"(state));
#endif
}

/**@brief       Get the interrupt priority state
 * @param       state
 *              Pointer to state variable where to store enabled interrupts
 * @inline
 */
static PORT_C_INLINE_ALWAYS void portIntPrioGet_(
    portReg_T *         state) {

    portReg_T           tmp;

#if (0 != PORT_CFG_MAX_ISR_PRIO)
    __asm __volatile__ (
        "   mrs     %0, basepri                             \n"
        : "=r"(tmp));
#else
    __asm __volatile__ (
        "   mrs     %0, primask                             \n"
        : "=r"(tmp));
#endif
    *state = tmp;
}

/**@brief       Get current and set new interrupt priority mask
 * @return      Current interrupt priority mask
 * @inline
 */
static PORT_C_INLINE_ALWAYS void portIntPrioReplace_(
    portReg_T *         old,
    portReg_T           new) {

    portReg_T           tmp;

#if (0 != PORT_CFG_MAX_ISR_PRIO)
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

/**@brief       Check if this is the last ISR executing
 * @return      Is the currently executed ISR the last one?
 *  @retval     TRUE - this is last ISR
 *  @retval     FALSE - this is not the last ISR
 * @inline
 */
static PORT_C_INLINE_ALWAYS bool_T portIsrIsLast_(
    void) {

    bool_T              ans;

    if (0u != (*CPU_SCB_ICSR & CPU_SCB_ICSR_RETTOBASE_MSK)) {
        ans = TRUE;
    } else {
        ans = FALSE;
    }

    return (ans);
}

/**@} *//*----------------------------------------------------------------*//**
 * @name        System timer management
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Initialize and start the system timer
 * @inline
 */
static PORT_C_INLINE_ALWAYS void portSysTmrInit_(
    portSysTmrReg_T     val) {

    *CPU_SYST_CSR &= ~CPU_SYST_CSR_ENABLE_MSK;                                  /* Disable SysTick Timer                                    */
    *CPU_SYST_RVR = val - 1u;                                                   /* Set SysTick reload register                              */
    *CPU_SYST_CVR = 0u;
    *CPU_SYST_CSR = CPU_SYST_CSR_CLKSOURCE_MSK;                                 /* SysTick uses the processor clock.                        */
}

/**@brief       Get current counter value
 * @inline
 */
static PORT_C_INLINE_ALWAYS portSysTmrReg_T portSysTmrGetCVal_(
    void) {

    return (*CPU_SYST_CVR);
}

/**@brief       Get reload counter value
 * @inline
 */
static PORT_C_INLINE_ALWAYS portSysTmrReg_T portSysTmrGetRVal_(
    void) {

    return (*CPU_SYST_RVR);
}

/**@brief       Load the system timer Reload value register
 * @inline
 */
static PORT_C_INLINE_ALWAYS void portSysTmrRld_(
    portSysTmrReg_T val) {

    --val;
    *CPU_SYST_CSR &= ~CPU_SYST_CSR_ENABLE_MSK;
    *CPU_SYST_RVR = val;
    *CPU_SYST_CVR = 0u;
    *CPU_SYST_CSR |= CPU_SYST_CSR_ENABLE_MSK;
}

/**@brief       Enable the system timer
 * @inline
 */
static PORT_C_INLINE_ALWAYS void portSysTmrEnable_(
    void) {

    *CPU_SYST_CSR |= CPU_SYST_CSR_ENABLE_MSK;
}

/**@brief       Disable the system timer
 * @inline
 */
static PORT_C_INLINE_ALWAYS void portSysTmrDisable_(
    void) {

    *CPU_SYST_CSR &= ~CPU_SYST_CSR_ENABLE_MSK;
}

/**@brief       Disable the system timer interrupt
 * @inline
 */
static PORT_C_INLINE_ALWAYS void portSysTmrIsrEnable_(
    void) {

    *CPU_SYST_CSR |= CPU_SYST_CSR_TICKINT_MSK;
}

/**@brief       Enable the system timer interrupt
 * @inline
 */
static PORT_C_INLINE_ALWAYS void portSysTmrIsrDisable_(
    void) {

    *CPU_SCB_ICSR |= CPU_SCB_ICSR_PENDSTCLR_MSK;
    *CPU_SYST_CSR &= ~CPU_SYST_CSR_TICKINT_MSK;
}

/**@} *//*----------------------------------------------------------------*//**
 * @name        Scheduler support
 * @{ *//*--------------------------------------------------------------------*/

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
static PORT_C_INLINE_ALWAYS uint_fast8_t portBitFindLastSet_(
    portReg_T       value) {

    uint_fast8_t    clz;

    __asm__ __volatile__ (
        "   clz    %0, %1                                   \n"
        : "=r"(clz)
        : "r"(value));

    return (31u - clz);
}

/**@} *//*----------------------------------------------------------------*//**
 * @name        Dispatcher context switching
 * @{ *//*--------------------------------------------------------------------*/

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
void * portCtxInit(
    void *          stck,
    size_t          stckSize,
    void (* fn)(void *),
    void *          arg);

/**@brief       Do the context switch
 * @details     This function will just initiate PendSV exception which will do
 *              the actual context switch
 * @inline
 */
static PORT_C_INLINE_ALWAYS void portCtxSw_(
    void) {

    *CPU_SCB_ICSR |= CPU_SCB_ICSR_PENDSVSET_MSK;
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
PORT_C_NORETURN void portCtxSwStart(
    void);

/** @} *//*---------------------------------------------------------------*//**
 * @name        Generic port functions
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Initialize port
 * @details     Function will set up sub-priority bits to zero and handlers
 *              interrupt priority.
 */
void portCpuInit(
    void);

/**@brief       Terminate port
 */
void portCpuTerm(
    void);

/** @} *//*---------------------------------------------------------------*//**
 * @name        Port specific functions
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Pop the first thread stack
 * @details     During the thread initialization a false stack was created
 *              mimicking the real interrupt stack described in @ref portCtx.
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

#if (CPU_DEF_SYSTMR_MAX_VAL < PORT_DEF_SYSTMR_ONE_TICK)
# error "eSolid RT Kernel port: System Timer overflow, please check CFG_SYSTMR_CLOCK_FREQUENCY and CFG_SYSTMR_EVENT_FREQUENCY options."
#endif

/** @endcond *//** @} *//******************************************************
 * END of cpu.h
 ******************************************************************************/
#endif /* CPU_H__ */
