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
 *//***********************************************************************//**
 * @file
 * @author      Nenad Radulovic
 * @brief       Implementation of ARM Cortex-M3 interrupt port.
 * @addtogroup  arm-none-eabi-gcc-v7-m_impl
 *********************************************************************//** @{ */

/*=========================================================  INCLUDE FILES  ==*/

#include <stddef.h>

#include "plat/compiler.h"
#include "arch/ncore.h"

/*=========================================================  LOCAL MACRO's  ==*/
/*======================================================  LOCAL DATA TYPES  ==*/

enum interrupt_no
{
    NONMASKABLEINT_IRQN   = -14,                                                /**< @brief 2 Non Maskable Interrupt                        */
    MEMORYMANAGEMENT_IRQN = -12,                                                /**< @brief 4 Cortex-M3 Memory Management Interrupt         */
    BUSFAULT_IRQN         = -11,                                                /**< @brief 5 Cortex-M3 Bus Fault Interrupt                 */
    USAGEFAULT_IRQN       = -10,                                                /**< @brief 6 Cortex-M3 Usage Fault Interrupt               */
    SVCALL_IRQN           = -5,                                                 /**< @brief 11 Cortex-M3 SV Call Interrupt                  */
    PENDSV_IRQN           = -2,                                                 /**< @brief 14 Cortex-M3 Pend SV Interrupt                  */
    SYSTIMER_IRQN         = -1                                                  /**< @brief 15 Cortex-M3 System Tick Interrupt              */
};

/*=============================================  LOCAL FUNCTION PROTOTYPES  ==*/


static void module_init_cpu(void);



static void module_term_cpu(void);



static void module_init_isr(void);



static void module_term_isr(void);



static void isr_set_priority_grouping(
    uint32_t                    grouping);



static void isr_set_priority(
    enum interrupt_no           intr_no,
    uint32_t                    priority);



static void module_init_core_timer(void);



static void module_term_core_timer(void);

/*=======================================================  LOCAL VARIABLES  ==*/

static void (* g_core_timer_handler[CONFIG_CORE_TIMER_HANDLERS])(void);

/*======================================================  GLOBAL VARIABLES  ==*/
/*============================================  LOCAL FUNCTION DEFINITIONS  ==*/


/**@brief       Initializes CPU module
 */
static void module_init_cpu(void)
{
    __asm__ __volatile__(
        "@  ncpu_init                                       \n"
        "   clrex                                           \n");
                                              /* Clear the exclusive monitor. */
}



/**@brief       Terminate CPU module
 */
static void module_term_cpu(void)
{
    ncpu_stop();
}



static void module_init_isr(void)
{
    nisr_disable();
    isr_set_priority_grouping(PORT_CONFIG_ISR_SUBPRIORITY);
                                                  /* Setup priority subgroup. */
    isr_set_priority(PENDSV_IRQN,   NISR_PRIO_TO_CODE(CONFIG_ISR_MAX_PRIO));
    isr_set_priority(SYSTIMER_IRQN, NISR_PRIO_TO_CODE(CONFIG_ISR_MAX_PRIO));
}



static void module_term_isr(void)
{
    nisr_disable();
}



/**@brief       Set Priority Grouping
 * @param       grouping
 *              Priority grouping field.
 * @details     The function sets the priority grouping field using the required
 *              unlock sequence. The parameter grouping is assigned to the field
 *              SCB->AIRCR [10:8] PRIGROUP field. Only values from 0..7 are used.
 *              In case of a conflict between priority grouping and available
 *              priority bits (PORT_ISR_PRIO_BITS), the smallest possible
 *              priority group is set.
 */
static void isr_set_priority_grouping(
    uint32_t                    grouping)
{
    ncpu_reg                    reg;

    grouping &= 0x07u;
    reg  = PORT_SCB->AIRCR;
    reg &= ~(PORT_SCB_AIRCR_VECTKEY_Msk | PORT_SCB_AIRCR_PRIGROUP_Msk);
    reg |=  (PORT_SCB_AIRCR_VECTKEY_VALUE << PORT_SCB_AIRCR_VECTKEY_Pos);
    reg |=  (grouping << PORT_SCB_AIRCR_PRIGROUP_Pos);
    PORT_SCB->AIRCR = reg;
}



/**@brief       Set Priority for Cortex-M  System Interrupts
 * @param       intrNum
 *              Interrupt number
 * @param       priority
 *              The priority of specified interrupt source. The parameter
 *              priority must be encoded with @ref ES_INTR_PRIO_TO_CODE.
 */
static void isr_set_priority(
    enum interrupt_no           intr_no,
    uint32_t                    priority)
{
    PORT_SCB->SHP[((uint32_t)(intr_no) & 0x0ful) - 4u] = (uint8_t)priority;
}



static void module_init_core_timer(void)
{
    ncore_timer_disable();
    ncore_timer_isr_disable();
    /*
     * TODO: Clear interrupt flag and interrupt enable bits
     * TODO: Set up ISR priority
     */
}



static void module_term_core_timer(void)
{
    ncore_timer_isr_disable();
    ncore_timer_disable();
    /*
     * TODO: Clear interrupt flag and interrupt enable bits
     */
}

/*===================================  GLOBAL PRIVATE FUNCTION DEFINITIONS  ==*/
/*====================================  GLOBAL PUBLIC FUNCTION DEFINITIONS  ==*/


void ncore_init(
    void)
{
    module_init_cpu();
    module_init_isr();
    module_init_core_timer();
}



void ncore_term(
    void)
{
    module_term_core_timer();
    module_term_isr();
    module_term_cpu();
}



void ntimer_set_handler(
    void                     (* handler)(void),
    uint_fast8_t                slot)
{
    g_core_timer_handler[slot] = handler;
}



void ncore_timer_isr(void)
{
    uint_fast8_t                count;

    for (count = 0; count < CONFIG_CORE_TIMER_HANDLERS; count++) {
        if (g_core_timer_handler[count] != NULL) {
            g_core_timer_handler[count]();
        }
    }
    /*
     * TODO: Clear interrupt flag
     */
}



void ncore_kernel_isr(void)
{

}



/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of nport.c
 ******************************************************************************/
