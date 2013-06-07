/******************************************************************************
 * This file is part of eSolid-Kernel
 *
 * Copyright (C) 2011, 2012 - Nenad Radulovic
 *
 * eSolid-Kernel is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * eSolid-Kernel is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with eSolid-Kernel; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 *
 * web site:    http://blueskynet.dyndns-server.com
 * e-mail  :    blueskyniss@gmail.com
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

#define R__                             volatile const                          /**< @brief Defines 'read only' permissions                 */
#define RW_                             volatile                                /**< @brief Defines 'read/write' permissions                */

#define PSR_THUMB_STATE_Pos             24                                      /**< @brief PSR Thumb state: Position                       */
#define PSR_THUMB_STATE_MSK             (1UL << PSR_THUMB_STATE_Pos)            /**< @brief PSR Thumb state: Mask                           */

/**@brief       Exception return value
 * @details     This value must is set to:
 *              - Exception return gets multiOcc from the process stck, thread
 *              mode
 */
#define EXC_RETURN                      0xFFFFFFFDUL

/**@brief       On AIRCR register writes, write 0x5FA, otherwise the write is
 *              ignored
 */
#define SCB_AIRCR_VECTKEY               0x5FAUL

/**@brief       This field determines the split of group priority from
 *              subpriority.
 * @warning     Change this value only if you are familiar with Cortex interrupt
 *              priority system and how kernel protects its critical sections.
 */
#define CPU_SCB_AIRCR_PRIGROUP          0U

/**@brief       This is the data that will be placed on task context at its
 *              creation
 * @details     This macro can be used if you need to specify different settings
 *              for Interruptible-continuable instructions. The setting is done
 *              in PSR register.
 */
#define CPU_PSR_DATA                    0UL

/** @} *//*---------------------------------------------------------------*//**
 * @name        System Control Block (SCB)
 * @{ *//*--------------------------------------------------------------------*/

#define SCB                             ((scb_T *)CPU_SCB_BASE)                 /**< @brief SCB configuration struct                        */

#define SCB_VTOR                        (CPU_SCB_BASE + 0x08UL)                 /**< @brief SCB Vector table offset register                */

#define SCB_AIRCR_PRIGROUP_POS          8                                       /**< @brief SCB aircr: PRIGROUP Position                    */
#define SCB_AIRCR_PRIGROUP_MSK          (7UL << SCB_AIRCR_PRIGROUP_POS)         /**< @brief SCB aircr: PRIGROUP Mask                        */
#define SCB_AIRCR_VECTKEY_POS           16                                      /**< @brief SCB aircr: VECTKEY Position                     */
#define SCB_AIRCR_VECTKEY_MSK           (0xFFFFUL << SCB_AIRCR_VECTKEY_POS)     /**< @brief SCB aircr: VECTKEY Mask                         */

#define SCB_CCR_STKALIGN_POS            9                                       /**< @brief SCB ccr: STKALIGN Position                      */
#define SCB_CCR_STKALIGN_MSK            (1UL << SCB_CCR_STKALIGN_POS)           /**< @brief SCB ccr: STKALIGN Mask                          */

/** @} *//*---------------------------------------------------------------*//**
 * @name        System Timer (SYSTMR)
 * @{ *//*--------------------------------------------------------------------*/

#define SYST                            ((sysTick_T *)CPU_SYST_BASE)            /**< @brief SYSTMR configuration struct                     */

#define SYST_CSR_COUNTFLAG_POS          16                                      /**< @brief SYSTMR csr: COUNTFLAG Position                  */
#define SYST_CSR_COUNTFLAG_MSK          (1UL << SYST_CSR_COUNTFLAG_POS)         /**< @brief SYSTMR csr: COUNTFLAG Mask                      */

#define SYST_CSR_CLKSOURCE_POS          2                                       /**< @brief SYSTMR csr: CLKSOURCE Position                  */
#define SYST_CSR_CLKSOURCE_MSK          (1UL << SYST_CSR_CLKSOURCE_POS)         /**< @brief SYSTMR csr: CLKSOURCE Mask                      */

#define SYST_CSR_ENABLE_POS             0                                       /**< @brief SYSTMR csr: ENABLE Position                     */
#define SYST_CSR_ENABLE_MSK             (1UL << SYST_CSR_ENABLE_POS)            /**< @brief SYSTMR csr: ENABLE Mask                         */

/** @} *//*-------------------------------------------------------------------*/
/*======================================================  LOCAL DATA TYPES  ==*/

/**@brief       Interrupt Number Definition
 * @details     Cortex-M3 Processor Exceptions Numbers
 */
typedef enum irqN {
    NONMASKABLEINT_IRQN   = -14,                                                /**< @brief 2 Non Maskable Interrupt                        */
    MEMORYMANAGEMENT_IRQN = -12,                                                /**< @brief 4 Cortex-M3 Memory Management Interrupt         */
    BUSFAULT_IRQN         = -11,                                                /**< @brief 5 Cortex-M3 Bus Fault Interrupt                 */
    USAGEFAULT_IRQN       = -10,                                                /**< @brief 6 Cortex-M3 Usage Fault Interrupt               */
    SVCALL_IRQN           = -5,                                                 /**< @brief 11 Cortex-M3 SV Call Interrupt                  */
    PENDSV_IRQN           = -2,                                                 /**< @brief 14 Cortex-M3 Pend SV Interrupt                  */
    SYST_IRQN             = -1                                                  /**< @brief 15 Cortex-M3 System Tick Interrupt              */
} irqN_T;

/**@brief       Structure type to access the System Control Block (SCB).
 */
typedef struct scb {
    R__ uint32_t    cpuid;                                                      /**< @brief cpuid Base Register                                   */
    RW_ uint32_t    icsr;                                                       /**< @brief Interrupt Control and State Register                  */
    RW_ uint32_t    vtor;                                                       /**< @brief Vector Table Offset Register                          */
    RW_ uint32_t    aircr;                                                      /**< @brief Application Interrupt and Reset Control Register      */
    RW_ uint32_t    scr;                                                        /**< @brief System Control Register                               */
    RW_ uint32_t    ccr;                                                        /**< @brief Configuration Control Register                        */
    RW_ uint8_t     shp[12];                                                    /**< @brief System Handlers Priority Registers (4-7, 8-11, 12-15) */
    RW_ uint32_t    shcsr;                                                      /**< @brief System Handler Control and State Register             */
    RW_ uint32_t    cfsr;                                                       /**< @brief Configurable Fault Status Register                    */
    RW_ uint32_t    hfsr;                                                       /**< @brief HardFault Status Register                             */
    RW_ uint32_t    dfsr;                                                       /**< @brief Debug Fault Status Register                           */
    RW_ uint32_t    mmfar;                                                      /**< @brief MemManage Fault Address Register                      */
    RW_ uint32_t    bfar;                                                       /**< @brief BusFault Address Register                             */
    RW_ uint32_t    afsr;                                                       /**< @brief Auxiliary Fault Status Register                       */
    R__ uint32_t    pfr[2];                                                     /**< @brief Processor Feature Register                            */
    R__ uint32_t    dfr;                                                        /**< @brief Debug Feature Register                                */
    R__ uint32_t    adr;                                                        /**< @brief Auxiliary Feature Register                            */
    R__ uint32_t    mmfr[4];                                                    /**< @brief Memory Model Feature Register                         */
    R__ uint32_t    isar[5];                                                    /**< @brief Instruction Set Attributes Register                   */
    uint32_t        RESERVED0[5];                                               /**< @brief Reserved                                              */
    RW_ uint32_t    cpacr;                                                      /**< @brief Coprocessor Access Control Register                   */
} scb_T;

/**@brief       Structure type to access the System Timer (SYSTMR).
 */
typedef struct sysTick {
    RW_ uint32_t    csr;                                                        /**< @brief SYSTMR Control and Status Register              */
    RW_ uint32_t    rvr;                                                        /**< @brief SYSTMR Reload Value Register                    */
    RW_ uint32_t    cvr;                                                        /**< @brief SYSTMR Current Value Register                   */
    R__ uint32_t    calib;                                                      /**< @brief SYSTMR Calibration Register                     */
} sysTick_T;

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
 * @param       prio
 *              Priority to set.
 * @details     The function sets the priority of an interrupt.
 * @note        The priority cannot be set for every core interrupt.
 */
static PORT_C_INLINE void intPrioSet(
    irqN_T          irqN,
    uint32_t        prio);

/*=======================================================  LOCAL VARIABLES  ==*/
/*======================================================  GLOBAL VARIABLES  ==*/
/*============================================  LOCAL FUNCTION DEFINITIONS  ==*/

static void threadExit(
    void) {

    esThdTerm(
        esThdGetId());

    while (TRUE) {
        ;
    }
}

static PORT_C_INLINE void intPrioSet(
    irqN_T          irqN,
    uint32_t        prio) {

    SCB->shp[((uint32_t)(irqN) & 0x0FUL) - 4U] = prio << (8U - CPU_ISR_PRIO_BITS); /* set Priority for Cortex-M  System Interrupts          */
}

/*===================================  GLOBAL PRIVATE FUNCTION DEFINITIONS  ==*/
/*====================================  GLOBAL PUBLIC FUNCTION DEFINITIONS  ==*/

void portSysTmrInit_(
    void) {

    SYST->csr &= ~SYST_CSR_ENABLE_MSK;                                          /* Disable SYST Timer                                       */
    SYST->rvr = PORT_SYSTMR_RELOAD_VAL - 1U;                                    /* set systick reload register                              */
    SYST->cvr = SYST->rvr;
    SYST->csr = SYST_CSR_CLKSOURCE_MSK;                                         /* SysTick uses the processor clock.                        */
}

void portSysTmrTerm_(
    void) {

    SYST->csr &= ~SYST_CSR_ENABLE_MSK;                                          /* Disable SYST Timer                                       */
}

void portSysTmrEnable_(
    void) {

    SYST->csr |= SYST_CSR_ENABLE_MSK;                                           /* Enable SYST Timer                                        */
}

void portSysTmrReload_(
    esTick_T      ticks) {

    SYST->csr &= ~SYST_CSR_ENABLE_MSK;                                          /* Disable SYST Timer                                       */
    SYST->rvr = (PORT_SYSTMR_RELOAD_VAL * ticks) - 1U;
    SYST->cvr = SYST->rvr;
    SYST->csr |= SYST_CSR_ENABLE_MSK;                                           /* Enable SYST Timer                                        */
}

/*
 * - 1-4    get the beginning of MSP stack from SCB->vtor register
 * - 5      enable interrupts so we can call SVC 0
 * - 8      initiate SVC 0 which switches the context to the first thread
 * - 10     infinite loop: will never reach here
 */
PORT_C_NORETURN void portThdStart_(
    void) {

    __asm__ __volatile__ (
        "   ldr     r0, =%0                                 \n\t"               /* (1)                                                      */
        "   ldr     r0, [r0]                                \n\t"               /* (2)                                                      */
        "   ldr     r0, [r0]                                \n\t"               /* (3)                                                      */
        "   msr     msp, r0                                 \n\t"               /* (4)                                                      */
        "   mov     r0, #0                                  \n\t"               /* (5)                                                      */
        "   msr     basepri, r0                             \n\t"               /* (6)                                                      */
        "   cpsie   i                                       \n\t"               /* (7)                                                      */
        "   svc     0                                       \n\t"               /* (8)                                                      */
        :
        : "i"(SCB_VTOR)
        : "sp", "r0", "memory");

    while (TRUE) {
        ;
    }
}

void * portCtxInit_(
    void *          stck,
    size_t          stckSize,
    void (* thread)(void *),
    void *          arg) {

    struct portCtx * sp;

    sp = (struct portCtx *)((uint8_t *)stck + stckSize);
    sp--;
    sp->xpsr = (portReg_T)(PSR_THUMB_STATE_MSK | CPU_PSR_DATA);
    sp->pc = (portReg_T)thread;
    sp->lr = (portReg_T)threadExit;
    sp->r0 = (portReg_T)arg;

    return (sp);
}

void portInitEarly_(
    void) {

    PORT_HWREG_SET(
        SCB->aircr,
        SCB_AIRCR_VECTKEY_MSK | SCB_AIRCR_PRIGROUP_MSK,
        (SCB_AIRCR_VECTKEY << SCB_AIRCR_VECTKEY_POS) |
           (CPU_SCB_AIRCR_PRIGROUP << SCB_AIRCR_PRIGROUP_POS));                 /* Setup priority subgroup to zero bits                     */
    intPrioSet(
        PENDSV_IRQN,
        CFG_CRITICAL_PRIO);
    intPrioSet(
        SVCALL_IRQN,
        CFG_CRITICAL_PRIO);
    intPrioSet(
        SYST_IRQN,
        CFG_CRITICAL_PRIO);
}

/*
 * - 2-4    lock the interrupts (by setting higher priority)
 * - 5      get the pointer to new thread ID structure in register r1
 * - 6      get the pointer to process stack
 * - 7      pop the thread stack from PSP stack
 * - 8      save thread top of stack to PSP register
 * - 9      restore previous interrupt priority
 * - 10     Load LR wih @ref EXC_RETURN value indicating that we want to
 *          return to thread mode and on return use the PSP stack
 */
PORT_C_NAKED void portSVC(
    void) {

#if (0 != CFG_CRITICAL_PRIO)
    __asm__ __volatile__ (
        "   ldr     r0, =%0                                 \n\t"               /* (1) Load gKernCtrl.cthd address                          */
        "   mov     r1, %2                                  \n\t"               /* (2)                                                      */
        "   mrs     r3, basepri                             \n\t"               /* (3)                                                      */
        "   msr     basepri, r1                             \n\t"               /* (4)                                                      */
        "   ldr     r1, [r0]                                \n\t"               /* (5)                                                      */
        "   ldr     r1, [r1]                                \n\t"               /* (6)                                                      */
        "   ldmia   r1!, {r4-r11}                           \n\t"               /* (7)                                                      */
        "   msr     psp, r1                                 \n\t"               /* (8)                                                      */
        "   msr     basepri, r3                             \n\t"               /* (9)                                                      */
        "   mov     lr, %1                                  \n\t"               /* (10)                                                     */
        "   bx      lr                                      \n\t"               /* Return to first thread                                   */
        :
        :   "i"(&gKernCtrl.cthd),
            "i"(EXC_RETURN),
            "i"(CPU_ISR_PRIO));
#else
    __asm__ __volatile__ (
        "   ldr     r0, =%0                                 \n\t"               /* (1) Load gKernCtrl.cthd address                          */
        "   cpsid   i                                       \n\t"               /* (2)                                                      */
        "   ldr     r1, [r0]                                \n\t"               /* (5)                                                      */
        "   ldr     r1, [r1]                                \n\t"               /* (6)                                                      */
        "   ldmia   r1!, {r4-r11}                           \n\t"               /* (7)                                                      */
        "   msr     psp, r1                                 \n\t"               /* (8)                                                      */
        "   cpsie   i                                       \n\t"               /* (9)                                                      */
        "   mov     lr, %1                                  \n\t"               /* (10)                                                     */
        "   bx      lr                                      \n\t"               /* Return to first thread                                   */
        :
        :   "i"(&gKernCtrl.cthd),
            "i"(EXC_RETURN));
#endif
}

/*
 * - 2-4    lock the interrupts (by setting higher priority)
 * - 5,6    save the current context on PSP stack
 * - 1,7,8  save the thread top of stack into the first member of the thread ID
 *          structure
 * - 9-10   Make gKernCtrl.cthd == gKernCtrl.pthd
 * - 11-13  restore new context
 * - 14     restore previous interrupt priority from main stck
 * Note:    LR was already loaded with valid EXC_RETURN value
 */
PORT_C_NAKED void portPendSV(
    void) {

#if (0 != CFG_CRITICAL_PRIO)
    __asm__ __volatile__ (
        "   ldr     r0, =%0                                 \n\t"               /* (1) Get the address of gKernCtrl                         */
        "   mov     r1, %3                                  \n\t"               /* (2)                                                      */
        "   mrs     r3, basepri                             \n\t"               /* (3)                                                      */
        "   msr     basepri, r1                             \n\t"               /* (4)                                                      */
        "   mrs     r1, psp                                 \n\t"               /* (5)                                                      */
        "   stmdb   r1!, {r4-r11}                           \n\t"               /* (6)                                                      */
        "   ldr     r2, [r0, %1]                            \n\t"               /* (7)                                                      */
        "   str     r1, [r2]                                \n\t"               /* (8)                                                      */
        "   ldr     r2, [r0, %2]                            \n\t"               /* (9)                                                      */
        "   str     r2, [r0]                                \n\t"               /* (10)                                                     */
        "   ldr     r1, [r2]                                \n\t"               /* (11)                                                     */
        "   ldmia   r1!, {r4-r11}                           \n\t"               /* (12)                                                     */
        "   msr     psp, r1                                 \n\t"               /* (13)                                                     */
        "   msr     basepri, r3                             \n\t"               /* (14)                                                     */
        "   bx      lr                                      \n\t"               /* Return to new thread                                     */
        :
        :   "i"(&gKernCtrl),
            "J"(offsetof(esKernCtrl_T, cthd)),
            "J"(offsetof(esKernCtrl_T, pthd)),
            "i"(CPU_ISR_PRIO));
#else
    __asm__ __volatile__ (
        "   ldr     r0, =%0                                 \n\t"               /* (1) Get the address of gCurrentThd                       */
        "   cpsid   i                                       \n\t"               /* (2)                                                      */
        "   mrs     r1, psp                                 \n\t"               /* (5)                                                      */
        "   stmdb   r1!, {r4-r11}                           \n\t"               /* (6)                                                      */
        "   ldr     r2, [r0, %1]                            \n\t"               /* (7)                                                      */
        "   str     r1, [r2]                                \n\t"               /* (8)                                                      */
        "   ldr     r2, [r0, %2]                            \n\t"               /* (9)                                                      */
        "   str     r2, [r0]                                \n\t"               /* (10)                                                     */
        "   ldr     r1, [r2]                                \n\t"               /* (11)                                                     */
        "   ldmia   r1!, {r4-r11}                           \n\t"               /* (12)                                                     */
        "   msr     psp, r1                                 \n\t"               /* (13)                                                     */
        "   cpsie   i                                       \n\t"               /* (14)                                                     */
        "   bx      lr                                      \n\t"               /* Return to new thread                                     */
        :
        :   "i"(&gKernCtrl),
            "J"(offsetof(esKernCtrl_T, cthd)),
            "J"(offsetof(esKernCtrl_T, pthd)));
#endif
}

void portSysTmr(
    void) {

    PORT_ISR_ENTER();
    esSysTmrHandlerI();
    PORT_ISR_EXIT();
}

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of cpu.c
 ******************************************************************************/
