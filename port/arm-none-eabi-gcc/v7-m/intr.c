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
 * @brief       Implementation of ARM Cortex-M3 interrupt port.
 * @addtogroup  arm-none-eabi-gcc-v7-m_impl
 *********************************************************************//** @{ */

/*=========================================================  INCLUDE FILES  ==*/

#include "plat/compiler.h"
#include "arch/intr.h"

/*=========================================================  LOCAL MACRO's  ==*/
/*======================================================  LOCAL DATA TYPES  ==*/
/*=============================================  LOCAL FUNCTION PROTOTYPES  ==*/

static PORT_C_INLINE void intrSetPriorityGrouping(
    uint32_t            grouping);

/*=======================================================  LOCAL VARIABLES  ==*/
/*======================================================  GLOBAL VARIABLES  ==*/
/*============================================  LOCAL FUNCTION DEFINITIONS  ==*/

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
static PORT_C_INLINE void intrSetPriorityGrouping(
    uint32_t            grouping) {

    grouping &= 0x07u;

    PORT_HWREG_SET(
        PORT_SCB->AIRCR,
        PORT_SCB_AIRCR_VECTKEY_Msk | PORT_SCB_AIRCR_PRIGROUP_Msk,
        (PORT_SCB_AIRCR_VECTKEY_VALUE << PORT_SCB_AIRCR_VECTKEY_Pos) |
           (grouping << PORT_SCB_AIRCR_PRIGROUP_Pos));
}

/*===================================  GLOBAL PRIVATE FUNCTION DEFINITIONS  ==*/
/*====================================  GLOBAL PUBLIC FUNCTION DEFINITIONS  ==*/

void portModuleIntrInit(
    void) {

    portIntrDisable_();
    intrSetPriorityGrouping(
        PORT_CONFIG_ISR_SUBPRIORITY);                                           /* Setup priority subgroup to zero bits                     */
}

void portModuleIntrTerm(
    void) {

    portIntrDisable_();
}

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of intr.c
 ******************************************************************************/
