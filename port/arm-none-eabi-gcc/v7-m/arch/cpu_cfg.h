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
 * @author  	Nenad Radulovic
 * @brief   	Configuration of CPU module.
 * @addtogroup  arm-none-eabi-gcc-v7-m_cfg
 * @brief		Configuration of CPU module.
 *********************************************************************//** @{ */

#if !defined(CPU_CFG_H__)
#define CPU_CFG_H__

/*=========================================================  INCLUDE FILES  ==*/

#include "family/profile.h"

/*===============================================================  DEFINES  ==*/
/*==============================================================  SETTINGS  ==*/

/**@brief       Priority of locked sections in kernel
 * @details     Specify the priority in range: <code>0</code>\f$\leq\f$
 *              <code>CFG_MAX_ISR_PRIO</code> \f$\leq\f$ <code>15</code>. The
 *              lower the number the higher the priority.
 * @note        When priority is set to @b 0 then critical code section will not
 *              use priority levels but instead it will just disable interrupts
 *              on entry and enable interrupts on exit.
 */
#if !defined(CFG_MAX_ISR_PRIO)
# define CFG_MAX_ISR_PRIO               7u
#endif

/**@brief       The frequency of clock which is used for the system timer
 * @details     System timer SysTick uses core clock (sometimes referred to as
 *              HCLK) for counting. Specify here the core clock so the OS can
 *              properly manage system tick event generation.
 */
#if !defined(CFG_SYSTMR_CLOCK_FREQUENCY)
# define CFG_SYSTMR_CLOCK_FREQUENCY     24000000ul
#endif

/**@brief       This is the data that will be placed on task context at its
 *              creation
 * @details     This macro can be used if you need to specify different settings
 *              for Interruptible-continuable instructions. The setting is done
 *              in PSR register.
 */
#define CFG_PSR_DATA                    0u

/**@brief       This field determines the split of group priority from
 *              subpriority.
 * @warning     Change this value only if you are familiar with Cortex interrupt
 *              priority system and how kernel protects its critical code
 *              sections.
 */
#define CFG_SCB_AIRCR_PRIGROUP          0u

/** @} *//*-------------------------------------------------------------------*/
/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of cpu_cfg.h
 ******************************************************************************/
#endif /* CPU_CFG_H__ */
