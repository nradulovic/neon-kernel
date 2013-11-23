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
 * @author  	Nenad Radulovic
 * @brief   	Configuration of Debug.
 * @addtogroup  template_dbg_cfg
 * @details     Each configuration option or setting has its own default value
 *              when not defined by the application. When application needs to
 *              change a setting it just needs to define a configuration macro
 *              with another value and the default configuration macro will be
 *              overridden.
 *********************************************************************//** @{ */

#if !defined(DBG_CFG_H_)
#define DBG_CFG_H_

/*=========================================================  INCLUDE FILES  ==*/

/*===============================================================  DEFINES  ==*/
/** @cond */

/** @endcond */
/*==============================================================  SETTINGS  ==*/

/**@brief       Enable/disable Debug module
 * @details     Possible values:
 *              - 0U - Debug is disabled
 *              - 1U - Debug is enabled
 */
#if !defined(CFG_DBG_ENABLE)
# define CFG_DBG_ENABLE                 1U
#endif

/**@brief       Enable/disable API arguments validation
 * @details     During the development cycle of the application this option
 *              should be turned on. When this configuration option is turned on
 *              the kernel API functions will also check arguments passed to
 *              them. If an invalid argument is detected the execution of the
 *              application will stop and the user will be informed about the
 *              error condition.
 *
 *              Possible values:
 *              - 0U - API validation is disabled
 *              - 1U - API validation is enabled
 *
 * @note        1) The error checking use userAssert() hook function to provide
 *              the information about the error condition.
 * @note        2) This option is enabled only if @ref CFG_DBG_ENABLE is enabled,
 *              too.
 */
#if !defined(CFG_DBG_API_VALIDATION)
# define CFG_DBG_API_VALIDATION         1U
#endif

/**@brief       Enable/disable internal checks
 * @details     Possible values:
 *              - 0U - API validation is disabled
 *              - 1U - API validation is enabled
 * @note        This option is enabled only if @ref CFG_DBG_ENABLE is enabled,
 *              too.
 */
#if !defined(CFG_DBG_INTERNAL_CHECK)
# define CFG_DBG_INTERNAL_CHECK         1U
#endif

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/

#if ((1U != CFG_DBG_ENABLE) && (0U != CFG_DBG_ENABLE))
# error "eSolid RT Kernel: Configuration option CFG_DBG_ENABLE is out of range."
#endif

#if ((1U != CFG_DBG_API_VALIDATION) && (0U != CFG_DBG_API_VALIDATION))
# error "eSolid RT Kernel: Configuration option CFG_DBG_API_VALIDATION is out of range."
#endif

#if ((1U != CFG_DBG_INTERNAL_CHECK) && (0U != CFG_DBG_INTERNAL_CHECK))
# error "eSolid RT Kernel: Configuration option CFG_DBG_INTERNAL_CHECK is out of range."
#endif

/** @endcond *//** @} *//******************************************************
 * END of err_cfg.h
 ******************************************************************************/
#endif /* DBG_CFG_H_ */
