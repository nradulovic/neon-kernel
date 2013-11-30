/*
 * This file is part of eSolid.
 *
 * Copyright (C) 2010 - 2013 Nenad Radulovic
 *
 * eSolid is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * eSolid is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with eSolid.  If not, see <http://www.gnu.org/licenses/>.
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

#include "kernel/kernel.h"

/*=========================================================  LOCAL MACRO's  ==*/

/**@brief       Exception return value
 * @details     This value must is set to:
 *              - Exception return gets stack from the process stack, thread
 *              mode
 */
#define DEF_EXC_RETURN                0xfffffffdul

/*======================================================  LOCAL DATA TYPES  ==*/
/*=============================================  LOCAL FUNCTION PROTOTYPES  ==*/

/**@brief       Exit thread function
 * @details     This function is called when a thread terminates
 */
static void threadExit(
    void);

/*=======================================================  LOCAL VARIABLES  ==*/
/*======================================================  GLOBAL VARIABLES  ==*/
/*============================================  LOCAL FUNCTION DEFINITIONS  ==*/

static void threadExit(
    void) {

    esThdTerm(
        esThdGetId());

    while (TRUE);
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
        : "i"(&SYS_SCB->vtor)
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
    sp->xpsr = (portReg_T)(CPU_PSR_THUMB_STATE_Msk | KCORE_CFG_PSR_DATA);
    sp->pc   = (portReg_T)fn;
    sp->lr   = (portReg_T)threadExit;
    sp->r0   = (portReg_T)arg;

    return (sp);
}

void portKCoreInit(
    void) {

    intrPrioSet_(
        PENDSV_IRQN,
        PORT_DEF_MAX_ISR_PRIO);
    intrPrioSet_(
        SVCALL_IRQN,
        PORT_DEF_MAX_ISR_PRIO);
    intrPrioSet_(
        SYST_IRQN,
        PORT_DEF_MAX_ISR_PRIO);
}

void portKCoreTerm(
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
PORT_C_NAKED void kcoreSVC(
    void) {

#if (0 != PORT_CFG_MAX_ISR_PRIO)
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
            "i"(PORT_DEF_MAX_ISR_PRIO));
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
PORT_C_NAKED void kcorePendSV(
    void) {

#if (0 != PORT_CFG_MAX_ISR_PRIO)
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
            "i"(PORT_DEF_MAX_ISR_PRIO));
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

    esKernIsrEnterI();
    esKernSysTmr();
    esKernIsrExitI();
}

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of cpu.c
 ******************************************************************************/
