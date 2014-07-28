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
 * @brief       Debug header
 * @defgroup    debug Debug
 * @brief       Debug
 *********************************************************************//** @{ */
/**@defgroup    debug_intf Interface
 * @brief       Public interface
 * @details     For more details see @ref errors_intro.
 * @{ *//*--------------------------------------------------------------------*/

#ifndef NDEBUG_H
#define NDEBUG_H

/*=========================================================  INCLUDE FILES  ==*/

#include <stdint.h>

#include "plat/compiler.h"
#include "nkernel_config.h"

/*===============================================================  MACRO's  ==*/

/*------------------------------------------------------------------------*//**
 * @name        Object and error source information
 * @brief       This describes the free object file and error source
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Declare a module information
 * @param       name
 *              Module name : string
 * @param       description
 *              Module description : string
 * @param       author
 *              Module author : string
 * @api
 */
#define NMODULE_INFO_CREATE(description, author)                                \
    PORT_C_ROM struct PORT_C_UNUSED nmodule_info g_module_info =                \
    {                                                                           \
        description,                                                            \
        author,                                                                 \
        PORT_C_FILE                                                             \
    }

/**@} *//*----------------------------------------------------------------*//**
 * @name        Error checking
 * @brief       These features are enabled/disabled using the option
 *              @ref CONFIG_DEBUG.
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Static assert macro
 * @param       msg
 *              Message : enum esDebugMessageNo : enumerated debug message.
 * @param       expr
 *              Expression : C expression : condition expression which must be
 *              TRUE.
 * @details     This macro will evaluate the given expression at compile time.
 *              If the expression is not true the compilation process will stop
 *              with an error message about negative size of array.
 * @api
 */
#if defined(PORT_C_STATIC_ASSERT)
# define NASSERT_STATIC(msg, expr)                                              \
    PORT_C_STATIC_ASSERT(expr)
#else
# define NASSERT_STATIC(msg, expr)                                              \
    extern char static_assert_has_failed_##msg[(expr) ? 1 : -1]
#endif

/**@brief       Generic assert macro.
 * @param       msg
 *              Message : enum esDebugMessageNo : enumerated debug message.
 * @param       expr
 *              Expression : C expression : condition expression which must be
 *              TRUE.
 * @api
 */
#if (CONFIG_DEBUG == 1)
# define NASSERT(msg, expr)                                                     \
    if (!(expr)) {                                                              \
        nassert(&g_module_info, PORT_C_FUNC, PORT_C_LINE, #expr, msg);          \
    }
#else
# define NASSERT(msg, expr)               (void)0
#endif

/**@brief       Assert macro that will always execute (no conditional).
 * @param       msg
 *              Message : enum esDebugMessageNo : enumerated debug message.
 * @param       text
 *              Text : string : a text which will be printed when this assert
 *              macro is executed.
 * @api
 */
#if (CONFIG_DEBUG == 1)
# define NASSERT_ALWAYS(text)                                                   \
    nassert(&g_module_info, PORT_C_FUNC, PORT_C_LINE, #text, NULL);
#else
# define NASSERT_ALWAYS(text)               (void)0
#endif

/**@} *//*----------------------------------------------------------------*//**
 * @name        Internal checking
 * @brief       These macros are enabled/disabled using the option
 *              @ref CONFIG_DEBUG_INTERNAL_CHECK.
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Assert macro used for internal execution checking
 * @param       msg
 *              Message : enum esDebugMessageNo : enumerated debug message.
 * @param       expr
 *              Expression : C expression : condition expression which must be
 *              satisfied
 * @api
 */
#if (CONFIG_DEBUG_INTERNAL == 1)
# define NASSERT_INTERNAL(msg, expr)        NASSERT(msg, expr)
#else
# define NASSERT_INTERNAL(msg, expr)        (void)0
#endif

/**@} *//*----------------------------------------------------------------*//**
 * @name        API contract validation
 * @brief       These macros are enabled/disabled using the option
 *              @ref CONFIG_DEBUG_API_VALIDATION.
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Execute code to fulfill the contract
 * @param       expr
 *              Expression : C expression : expression to be executed only if
 *              contracts need to be validated.
 * @api
 */
#if (CONFIG_DEBUG_API == 1)
# define NOBLIGATION(expr)                  expr
#else
# define NOBLIGATION(expr)                  (void)0
#endif

/**@brief       Make sure the caller has fulfilled all contract preconditions
 * @param       msg
 *              Message : enum esDebugMessageNo : enumerated debug message.
 * @param       expr
 *              Expression : C expression : condition expression which must be
 *              satisfied
 * @api
 */
#if (1 == CONFIG_DEBUG_API)
# define NREQUIRE(msg, expr)                NASSERT(msg, expr)
#else
# define NREQUIRE(msg, expr)                (void)0
#endif

/**@brief       Make sure the callee has fulfilled all contract postconditions
 * @param       expr
 *              Expression : C expression : condition expression which must be
 *              satisfied
 * @api
 */
#if    (CONFIG_DEBUG == 1)
# define NENSURE(expr)                                                          \
    if ((expr) != ES_ERROR_NONE) {                                              \
        nassert(&g_module_info, PORT_C_FUNC, PORT_C_LINE, #expr, NAPI_CALL);    \
    }
#else
# define NENSURE(expr)                      expr
#endif

/**@brief       Make sure the caller has fulfilled all contract preconditions
 * @param       msg
 *              Message : enum esDebugMessageNo : enumerated debug message.
 * @param       expr
 *              Expression : C expression : condition expression which must be
 *              satisfied
 * @api
 */
#if   (CONFIG_DEBUG_INTERNAL == 1) && (CONFIG_DEBUG_API == 1)
# define NREQUIRE_INTERNAL(msg, expr)       NASSERT(msg, expr)
#else
# define NREQUIRE_INTERNAL(msg, expr)       (void)0
#endif

/**@brief       Make sure the callee has fulfilled all contract postconditions
 * @param       expr
 *              Expression : C expression : condition expression which must be satisfied
 * @api
 */
#if   (CONFIG_DEBUG_INTERNAL == 1) && (CONFIG_DEBUG_API == 1)
# define NENSURE_INTERNAL(expr)             NENSURE(expr)
#else
# define NENSURE_INTERNAL(expr)             expr
#endif

#define NAPI_RANGE                          "Value is out of valid range."
#define NAPI_OBJECT                         "Object is not valid."
#define NAPI_POINTER                        "Pointer has null value."
#define NAPI_USAGE                          "Object/method usage failure."
#define NAPI_CALL                           "An API method call has failed."
#define ES_ASSERT_FAILED                    "Assert failed"

/**@} *//*-----------------------------------------------  C++ extern base  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/

/**@brief       Defines module information structure
 * @api
 */
struct nmodule_info
{
    const PORT_C_ROM char * const PORT_C_ROM_VAR desc;                          /**<@brief Module description         */
    const PORT_C_ROM char * const PORT_C_ROM_VAR auth;                          /**<@brief Module author              */
    const PORT_C_ROM char * const PORT_C_ROM_VAR file;                          /**<@brief Module source file         */
};

/*======================================================  GLOBAL VARIABLES  ==*/
/*===================================================  FUNCTION PROTOTYPES  ==*/

/**@brief       An assertion has failed
 * @param       module_info
 *              Object module information
 * @param       debug_object
 *              C Object describes where the error occurred.
 * @param       expr
 *              Expression: is pointer to the string containing the expression
 *              that failed to evaluate to `TRUE`.
 * @param       msg
 *              String containing some information about the error.
 * @note        1) This function may be called only if @ref CONFIG_DEBUG is
 *              active.
 * @details     Function will prepare the information which was given by the
 *              macros and pass the information to hook_at_failed_assert()
 *              function for presentation.
 * @notapi
 */
PORT_C_NORETURN void nassert(
    const PORT_C_ROM struct nmodule_info * module_info,
    const PORT_C_ROM char *     fn,
    uint32_t                    line,
    const PORT_C_ROM char *     expr,
    const PORT_C_ROM char *     msg);

/**@brief       An assertion has failed. This function should inform the user
 *              about failed assertion.
 * @param       debug_report
 *              Debug report: is pointer to the debug report created by
 *              ndebug_assert() function.
 * @note        1) The function may be called only if @ref CONFIG_DEBUG is
 *                  active.
 * @note        2) The function will be called with interrupts disabled.
 * @details     Function will just print the information which was given by the
 *              macros.
 */
extern void hook_at_failed_assert(
    const PORT_C_ROM struct nmodule_info * module_info,
    const PORT_C_ROM char *     fn,
    uint32_t                    line,
    const PORT_C_ROM char *     expr,
    const PORT_C_ROM char *     msg);

/*--------------------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//** @} *//*********************************************
 * END of ndebug.h
 ******************************************************************************/
#endif /* NDEBUG_H */
