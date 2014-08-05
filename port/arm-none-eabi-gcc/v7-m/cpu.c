/*
 * This file is part of nub-rt-kernel
 *
 * Template version: 1.1.16 (24.12.2013)
 *
 * Copyright (C) 2011, 2012 - Nenad Radulovic
 *
 * nub-rt-kernel is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * nub-rt-kernel is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with eSolid.  If not, see <http://www.gnu.org/licenses/>.
 *
 * web site:    http://blueskynet.dyndns-server.com
 * e-mail  :    blueskyniss@gmail.com
 *//***********************************************************************//**
 * @file
 * @author      nenad
 * @brief       Short desciption of file
 * @addtogroup  module_impl
 *********************************************************************//** @{ */

/*=========================================================  INCLUDE FILES  ==*/

#include "arch/cpu.h"
#include "arch/intr.h"
#include "nkernel.h"

/*=========================================================  LOCAL MACRO's  ==*/
/*======================================================  LOCAL DATA TYPES  ==*/
/*=============================================  LOCAL FUNCTION PROTOTYPES  ==*/
/*=======================================================  LOCAL VARIABLES  ==*/
/*======================================================  GLOBAL VARIABLES  ==*/
/*============================================  LOCAL FUNCTION DEFINITIONS  ==*/
/*===================================  GLOBAL PRIVATE FUNCTION DEFINITIONS  ==*/
/*====================================  GLOBAL PUBLIC FUNCTION DEFINITIONS  ==*/



/*
 * - 2-4    lock the interrupts (by setting higher priority)
 * - 5,6    save the current context on PSP stack
 * - 1,7,8  save the thread top of stack into the first member of the thread ID
 *          structure
 * - 9-10   Make g_nsched.cthd == g_nsched.pthd
 * - 11-13  restore new context
 * - 14     restore previous interrupt priority from main stack
 * Note:    LR was already loaded with valid DEF_EXC_RETURN value
 */
PORT_C_NAKED void port_pendsv(
    void)
{
#if (0 != CONFIG_INTR_MAX_PRIO)
    __asm__ __volatile__ (
        "@  port_pendsv                                     \n"
        "   mov     r1, %0                                  \n"                 /* (2)                                                      */
        "   mrs     r0, basepri                             \n"                 /* (3)                                                      */
        "   msr     basepri, r1                             \n"                 /* (4)                                                      */
        "   push    {r0, r1}                                \n"
        "   add     r0, sp, #(1 * 4)                        \n"
        "   mov     r3, #1                                  \n"
        "   lsls    r3, r3, #24                             \n"
        "   ldr     r2, .kernel                             \n"
        "   ldr     r1, .restore_context                    \n"
        "   push    {r1, r2, r3}                            \n"
        "   sub     sp, sp, #(4 * 4)                        \n"
        "   push    {r0}                                    \n"
        "   bx      lr                                      \n"                 /* Return to new thread                                     */
        ".restore_context:                                  \n"
        "   svc     #0                                      \n"
        ".kernel:                                           \n"
        "   .word nkernel_reschedule_i                      \n"
        :
        :  "Ir" (NINTR_PRIO_TO_CODE(CONFIG_INTR_MAX_PRIO))
        : "r0", "r1", "r2", "r3", "cc", "sp");
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
        :   "i"(&global_nsched),
            "J"(offsetof(esKernCtrl_T, current)),
            "J"(offsetof(esKernCtrl_T, pending)));
#endif
}

PORT_C_NAKED void port_svc(void)
{
    __asm__ __volatile__ (
        "@  port_svc                                        \n"
        "   add     sp, sp, #(8 * 4)                        \n"
        "   pop     {r0, r1}                                \n"
        "   msr     basepri, r0                             \n"                 /* (14)                                                     */
        "   bx      lr                                      \n");
}

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of cpu.c
 ******************************************************************************/
