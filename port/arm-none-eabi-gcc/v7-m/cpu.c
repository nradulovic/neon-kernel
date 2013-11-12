/*
 * This file is part of eSolid-Kernel
 *
 * Copyright (C) 2011, 2012, 2013 - Nenad Radulovic
 *
 * eSolid-Kernel is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option) any
 * later version.
 *
 * eSolid-Kernel is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * eSolid-Kernel; if not, write to the Free Software Foundation, Inc., 51
 * Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * web site:    http://github.com/nradulovic
 * e-mail  :    nenad.b.radulovic@gmail.com
 *//***********************************************************************//**
 * @file
 * @author      Nenad Radulovic
 * @brief       Implementation of ARM Cortex-M3 CPU port.
 * @addtogroup  arm-none-eabi-gcc-v7-m_impl
 *********************************************************************//** @{ */

/*=========================================================  INCLUDE FILES  ==*/

#include "kernel.h"

/*=========================================================  LOCAL MACRO's  ==*/

/*------------------------------------------------------------------------*//**
 * @name        Port specific constants
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Exception return value
 * @details     This value must is set to:
 *              - Exception return gets stack from the process stack, thread
 *              mode
 */
#define DEF_EXC_RETURN                0xfffffffdul

/**@brief       On AIRCR register writes, write 0x5FA, otherwise the write is
 *              ignored
 */
#define DEF_SCB_AIRCR_VECTKEY         0x5faul

/** @} *//*---------------------------------------------------------------*//**
 * @name        System Control Block (SCB)
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       SCB configuration struct.
 */
#define SCB                             ((volatile struct scb *)CPU_SCB_BASE)

/**@brief       SCB Vector table offset register
 */
#define SCB_VTOR                        (CPU_SCB_BASE + 0x08ul)

/**@brief       SCB aircr: PRIGROUP Position
 */
#define SCB_AIRCR_PRIGROUP_POS          (8ul)

/**@brief       SCB aircr: PRIGROUP Mask
 */
#define SCB_AIRCR_PRIGROUP_MSK          (7ul << SCB_AIRCR_PRIGROUP_POS)

/**@brief       SCB aircr: VECTKEY Position
 */
#define SCB_AIRCR_VECTKEY_POS           (16ul)

/**@brief       SCB aircr: VECTKEY Mask
 */
#define SCB_AIRCR_VECTKEY_MSK           (0xfffful << SCB_AIRCR_VECTKEY_POS)

/**@brief       SCB ccr: STKALIGN Position
 */
#define SCB_CCR_STKALIGN_POS            (9ul)

/**@brief       SCB ccr: STKALIGN Mask
 */
#define SCB_CCR_STKALIGN_MSK            (0x01ul << SCB_CCR_STKALIGN_POS)

/**@brief       PSR Thumb state: Position.
 */
#define PSR_THUMB_STATE_POS             (24u)

/**@brief       PSR Thumb state: Mask.
 */
#define PSR_THUMB_STATE_MSK             (0x01ul << PSR_THUMB_STATE_POS)

/*======================================================  LOCAL DATA TYPES  ==*/

/**@brief       Interrupt Number Definition
 * @details     Cortex-M3 Processor Exceptions Numbers
 */
enum irqN {
    NONMASKABLEINT_IRQN   = -14,                                                /**< @brief 2 Non Maskable Interrupt                        */
    MEMORYMANAGEMENT_IRQN = -12,                                                /**< @brief 4 Cortex-M3 Memory Management Interrupt         */
    BUSFAULT_IRQN         = -11,                                                /**< @brief 5 Cortex-M3 Bus Fault Interrupt                 */
    USAGEFAULT_IRQN       = -10,                                                /**< @brief 6 Cortex-M3 Usage Fault Interrupt               */
    SVCALL_IRQN           = -5,                                                 /**< @brief 11 Cortex-M3 SV Call Interrupt                  */
    PENDSV_IRQN           = -2,                                                 /**< @brief 14 Cortex-M3 Pend SV Interrupt                  */
    SYST_IRQN             = -1                                                  /**< @brief 15 Cortex-M3 System Tick Interrupt              */
};

/**@brief       Structure type to access the System Control Block (SCB).
 */
struct scb {
    uint32_t            cpuid;                                                  /**< @brief cpuid Base Register                             */
    uint32_t            icsr;                                                   /**< @brief Interrupt Control and State Register            */
    uint32_t            vtor;                                                   /**< @brief Vector Table Offset Register                    */
    uint32_t            aircr;                                                  /**< @brief Application Interrupt and Reset Control Register*/
    uint32_t            scr;                                                    /**< @brief System Control Register                         */
    uint32_t            ccr;                                                    /**< @brief Configuration Control Register                  */
    uint8_t             shp[12];                                                /**< @brief System Handlers Priority Registers              */
    uint32_t            shcsr;                                                  /**< @brief System Handler Control and State Register       */
    uint32_t            cfsr;                                                   /**< @brief Configurable Fault Status Register              */
    uint32_t            hfsr;                                                   /**< @brief HardFault Status Register                       */
    uint32_t            dfsr;                                                   /**< @brief Debug Fault Status Register                     */
    uint32_t            mmfar;                                                  /**< @brief MemManage Fault Address Register                */
    uint32_t            bfar;                                                   /**< @brief BusFault Address Register                       */
    uint32_t            afsr;                                                   /**< @brief Auxiliary Fault Status Register                 */
    uint32_t            pfr[2];                                                 /**< @brief Processor Feature Register                      */
    uint32_t            dfr;                                                    /**< @brief Debug Feature Register                          */
    uint32_t            adr;                                                    /**< @brief Auxiliary Feature Register                      */
    uint32_t            mmfr[4];                                                /**< @brief Memory Model Feature Register                   */
    uint32_t            isar[5];                                                /**< @brief Instruction Set Attributes Register             */
    uint32_t            RESERVED0[5];                                           /**< @brief Reserved                                        */
    uint32_t            cpacr;                                                  /**< @brief Coprocessor Access Control Register             */
};


/*=============================================  LOCAL FUNCTION PROTOTYPES  ==*/

/**@brief       Exit thread function
 * @details     This function is called when a thread terminates
 */
static void threadExit(
    void);

/**
 * @brief       Set Interrupt Priority
 * @param       irqN
 *              Interrupt number.
 * @details     The function sets the priority of an interrupt.
 * @note        The priority cannot be set for every core interrupt.
 */
static PORT_C_INLINE void intPrioSet(
    enum irqN           irqN,
    uint32_t            prio);

/*=======================================================  LOCAL VARIABLES  ==*/
/*======================================================  GLOBAL VARIABLES  ==*/
/*============================================  LOCAL FUNCTION DEFINITIONS  ==*/

static void threadExit(
    void) {

    esThdTerm(
        esThdGetId());

    while (TRUE);
}

static PORT_C_INLINE void intPrioSet(
    enum irqN           irqN,
    uint32_t            prio) {

    SCB->shp[((uint32_t)(irqN) & 0x0ful) - 4u] = prio;                          /* set Priority for Cortex-M  System Interrupts          */
}

/*===================================  GLOBAL PRIVATE FUNCTION DEFINITIONS  ==*/
/*====================================  GLOBAL PUBLIC FUNCTION DEFINITIONS  ==*/

/*
 * - 1-4    get the beginning of MSP stack from SCB->vtor register
 * - 5      enable interrupts so we can call SVC 0
 * - 8      initiate SVC 0 which switches the context to the first thread
 * - 10     infinite loop: will never reach here
 */
PORT_C_NORETURN void portCtxSwStart(
    void) {

    __asm__ __volatile__ (
        "   ldr     r0, =%0                                 \n"               /* (1)                                                      */
        "   ldr     r0, [r0]                                \n"               /* (2)                                                      */
        "   ldr     r0, [r0]                                \n"               /* (3)                                                      */
        "   msr     msp, r0                                 \n"               /* (4)                                                      */
        "   mov     r0, #0                                  \n"               /* (5)                                                      */
        "   msr     basepri, r0                             \n"               /* (6)                                                      */
        "   cpsie   i                                       \n"               /* (7)                                                      */
        "   svc     0                                       \n"               /* (8)                                                      */
        :
        : "i"(SCB_VTOR)
        : "sp", "r0", "memory");

    while (TRUE);
}

void * portCtxInit(
    void *              stck,
    size_t              stckSize,
    void (* fn)(void *),
    void *              arg) {

    struct portCtx *    sp;

    sp       = (struct portCtx *)((uint8_t *)stck + stckSize);
    sp--;
    sp->xpsr = (portReg_T)(PSR_THUMB_STATE_MSK | CFG_PSR_DATA);
    sp->pc   = (portReg_T)fn;
    sp->lr   = (portReg_T)threadExit;
    sp->r0   = (portReg_T)arg;

    return (sp);
}

void portCpuInit(
    void) {

    PORT_HWREG_SET(
        SCB->aircr,
        SCB_AIRCR_VECTKEY_MSK | SCB_AIRCR_PRIGROUP_MSK,
        (DEF_SCB_AIRCR_VECTKEY << SCB_AIRCR_VECTKEY_POS) |
           (CFG_SCB_AIRCR_PRIGROUP << SCB_AIRCR_PRIGROUP_POS));                 /* Setup priority subgroup to zero bits                     */
    intPrioSet(
        PENDSV_IRQN,
        PORT_DEF_INT_PRIO);
    intPrioSet(
        SVCALL_IRQN,
        PORT_DEF_INT_PRIO);
    intPrioSet(
        SYST_IRQN,
        PORT_DEF_INT_PRIO);
}

void portCpuTerm(
    void) {

    /*
     * TODO: put CPU to sleep or for (;;)
     */
}

/*
 * - 2-4    lock the interrupts (by setting higher priority)
 * - 5      get the pointer to new thread ID structure in register r1
 * - 6      get the pointer to process stack
 * - 7      pop the thread stack from PSP stack
 * - 8      save thread top of stack to PSP register
 * - 9      restore previous interrupt priority
 * - 10     Load LR wih @ref DEF_EXC_RETURN value indicating that we want to
 *          return to thread mode and on return use the PSP stack
 */
PORT_C_NAKED void portSVC(
    void) {

#if (0 != CFG_MAX_ISR_PRIO)
    __asm__ __volatile__ (
        "   ldr     r0, =%0                                 \n"               /* (1) Load KernCtrl.cthd address                          */
        "   mov     r1, %2                                  \n"               /* (2)                                                      */
        "   mrs     r3, basepri                             \n"               /* (3)                                                      */
        "   msr     basepri, r1                             \n"               /* (4)                                                      */
        "   ldr     r1, [r0]                                \n"               /* (5)                                                      */
        "   ldr     r1, [r1]                                \n"               /* (6)                                                      */
        "   ldmia   r1!, {r4-r11}                           \n"               /* (7)                                                      */
        "   msr     psp, r1                                 \n"               /* (8)                                                      */
        "   msr     basepri, r3                             \n"               /* (9)                                                      */
        "   mov     lr, %1                                  \n"               /* (10)                                                     */
        "   bx      lr                                      \n"               /* Return to first thread                                   */
        :
        :   "i"(&KernCtrl.cthd),
            "i"(DEF_EXC_RETURN),
            "i"(PORT_DEF_INT_PRIO));
#else
    __asm__ __volatile__ (
        "   ldr     r0, =%0                                 \n"               /* (1) Load KernCtrl.cthd address                          */
        "   cpsid   i                                       \n"               /* (2)                                                      */
        "   ldr     r1, [r0]                                \n"               /* (5)                                                      */
        "   ldr     r1, [r1]                                \n"               /* (6)                                                      */
        "   ldmia   r1!, {r4-r11}                           \n"               /* (7)                                                      */
        "   msr     psp, r1                                 \n"               /* (8)                                                      */
        "   cpsie   i                                       \n"               /* (9)                                                      */
        "   mov     lr, %1                                  \n"               /* (10)                                                     */
        "   bx      lr                                      \n"               /* Return to first thread                                   */
        :
        :   "i"(&KernCtrl.cthd),
            "i"(DEF_EXC_RETURN));
#endif
}

/*
 * - 2-4    lock the interrupts (by setting higher priority)
 * - 5,6    save the current context on PSP stack
 * - 1,7,8  save the thread top of stack into the first member of the thread ID
 *          structure
 * - 9-10   Make KernCtrl.cthd == KernCtrl.pthd
 * - 11-13  restore new context
 * - 14     restore previous interrupt priority from main stck
 * Note:    LR was already loaded with valid DEF_EXC_RETURN value
 */
PORT_C_NAKED void portPendSV(
    void) {

#if (0 != CFG_MAX_ISR_PRIO)
    __asm__ __volatile__ (
        "   ldr     r0, =%0                                 \n"               /* (1) Get the address of KernCtrl                         */
        "   mov     r1, %3                                  \n"               /* (2)                                                      */
        "   mrs     r3, basepri                             \n"               /* (3)                                                      */
        "   msr     basepri, r1                             \n"               /* (4)                                                      */
        "   mrs     r1, psp                                 \n"               /* (5)                                                      */
        "   stmdb   r1!, {r4-r11}                           \n"               /* (6)                                                      */
        "   ldr     r2, [r0, %1]                            \n"               /* (7)                                                      */
        "   str     r1, [r2]                                \n"               /* (8)                                                      */
        "   ldr     r2, [r0, %2]                            \n"               /* (9)                                                      */
        "   str     r2, [r0]                                \n"               /* (10)                                                     */
        "   ldr     r1, [r2]                                \n"               /* (11)                                                     */
        "   ldmia   r1!, {r4-r11}                           \n"               /* (12)                                                     */
        "   msr     psp, r1                                 \n"               /* (13)                                                     */
        "   msr     basepri, r3                             \n"               /* (14)                                                     */
        "   bx      lr                                      \n"               /* Return to new thread                                     */
        :
        :   "i"(&KernCtrl),
            "J"(offsetof(struct kernCtrl_, cthd)),
            "J"(offsetof(struct kernCtrl_, pthd)),
            "i"(PORT_DEF_INT_PRIO));
#else
    __asm__ __volatile__ (
        "   ldr     r0, =%0                                 \n"               /* (1) Get the address of gCurrentThd                       */
        "   cpsid   i                                       \n"               /* (2)                                                      */
        "   mrs     r1, psp                                 \n"               /* (5)                                                      */
        "   stmdb   r1!, {r4-r11}                           \n"               /* (6)                                                      */
        "   ldr     r2, [r0, %1]                            \n"               /* (7)                                                      */
        "   str     r1, [r2]                                \n"               /* (8)                                                      */
        "   ldr     r2, [r0, %2]                            \n"               /* (9)                                                      */
        "   str     r2, [r0]                                \n"               /* (10)                                                     */
        "   ldr     r1, [r2]                                \n"               /* (11)                                                     */
        "   ldmia   r1!, {r4-r11}                           \n"               /* (12)                                                     */
        "   msr     psp, r1                                 \n"               /* (13)                                                     */
        "   cpsie   i                                       \n"               /* (14)                                                     */
        "   bx      lr                                      \n"               /* Return to new thread                                     */
        :
        :   "i"(&KernCtrl),
            "J"(offsetof(esKernCtrl_T, cthd)),
            "J"(offsetof(esKernCtrl_T, pthd)));
#endif
}

void portSysTmr(
    void) {

    esKernIsrEnter();
    esKernSysTmr();
    esKernIsrExit();
}

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of cpu.c
 ******************************************************************************/
