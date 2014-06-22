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
 * @brief   	Family port specific configuration options
 * @addtogroup  template_family_cfg
 * @brief       Family port specific configuration options
 *********************************************************************//** @{ */

#if !defined(PROFILE_H__)
#define PROFILE_H__

/*=========================================================  INCLUDE FILES  ==*/
/*===============================================================  DEFINES  ==*/
/*==============================================================  SETTINGS  ==*/

/*------------------------------------------------------------------------*//**
 * @name        Generic family defaults
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       System timer maximum value
 * @details     This macro specifies maximum value that can be reloaded into
 *              system timer counter. For example, if the system timer is an
 *              8-bit counter than this macro would have the value of 0xffu (255).
 */
#define CPU_DEF_SYSTMR_MAX_VAL          0xfful

/**@} *//*----------------------------------------------------------------*//**
 * @name        Exception handlers used by the port
 * @{ *//*--------------------------------------------------------------------*/

/**@} *//*--------------------------------------------------------------------*/
/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of profile.h
 ******************************************************************************/
#endif /* PROFILE_H__ */
