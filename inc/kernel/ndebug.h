/*
 * This file is part of nKernel.
 *
 * Copyright (C) 2010 - 2013 Nenad Radulovic
 *
 * nKernel is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * nKernel is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with nKernel.  If not, see <http://www.gnu.org/licenses/>.
 *
 * web site:    http://github.com/nradulovic
 * e-mail  :    nenad.b.radulovic@gmail.com
 *//***************************************************************************************************************//**
 * @file
 * @author      Nenad Radulovic
 * @brief       Debug header
 * @defgroup    debug Debug
 * @brief       Debug
 *************************************************************************************************************//** @{ */
/**@defgroup    debug_intf Interface
 * @brief       Public interface
 * @details     For more details see @ref errors_intro.
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

#ifndef NDEBUG_H
#define NDEBUG_H

/*=================================================================================================  INCLUDE FILES  ==*/

#include <stdint.h>

#include "plat/compiler.h"
#include "kernel/nkernel_config.h"

/*=======================================================================================================  MACRO's  ==*/

/*----------------------------------------------------------------------------------------------------------------*//**
 * @name        Object and error source information
 * @brief       This describes the free object file and error source
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

/**@brief       Declare a module information
 * @param       name
 *              Module name : string
 * @param       description
 *              Module description : string
 * @param       author
 *              Module author : string
 * @api
 */
#define NMODULE_INFO_CREATE(name, description, author)                                                                  \
    PORT_C_ROM struct PORT_C_UNUSED nmodule_info global_module_info =                                                   \
    {                                                                                                                   \
        name,                                                                                                           \
        description,                                                                                                    \
        author,                                                                                                         \
        PORT_C_FILE                                                                                                     \
    }

/**@} *//*--------------------------------------------------------------------------------------------------------*//**
 * @name        Error checking
 * @brief       These features are enabled/disabled using the option @ref CONFIG_DEBUG.
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

/**@brief       Static assert macro
 * @param       msg
 *              Message : enum esDebugMessageNo : enumerated debug message.
 * @param       expr
 *              Expression : C expression : condition expression which must be TRUE.
 * @details     This macro will evaluate the given expression at compile time. If the expression is not true the
 *              compilation process will stop with an error message about negative size of array.
 * @api
 */
#if defined(PORT_C_STATIC_ASSERT)
# define NASSERT_STATIC(msg, expr)          PORT_C_STATIC_ASSERT(expr)
#else
# define NASSERT_STATIC(msg, expr)          extern char static_assert_has_failed_##msg[(expr) ? 1 : -1]
#endif

/**@brief       Generic assert macro.
 * @param       msg
 *              Message : enum esDebugMessageNo : enumerated debug message.
 * @param       expr
 *              Expression : C expression : condition expression which must be TRUE.
 * @api
 */
#if (1 == CONFIG_DEBUG)
# define NASSERT(msg, expr)                                                                                             \
    do                                                                                                                  \
    {                                                                                                                   \
        if (!(expr))                                                                                                    \
        {                                                                                                               \
            static const PORT_C_ROM struct ndebug_object this_object =                                                  \
            {                                                                                                           \
                &global_module_info,                                                                                    \
                PORT_C_FUNC,                                                                                            \
                PORT_C_LINE                                                                                             \
            };                                                                                                          \
            ndebug_assert(&this_object, #expr, msg);                                                                    \
        }                                                                                                               \
    }                                                                                                                   \
    while (0u)
#else
# define NASSERT(msg, expr)               (void)0
#endif

/**@brief       Generic assert macro (anonymous).
 * @param       msg
 *              Message : enum esDebugMessageNo : enumerated debug message.
 * @param       expr
 *              Expression : C expression : condition expression which must be TRUE.
 * @api
 */
#if (1 == CONFIG_DEBUG)
# define NASSERT_NONAME(msg, expr)                                                                                      \
    do                                                                                                                  \
    {                                                                                                                   \
        if (!(expr))                                                                                                    \
        {                                                                                                               \
            static const PORT_C_ROM struct ndebug_object this_object =                                                  \
            {                                                                                                           \
                &global_module_noname,                                                                                  \
                PORT_C_FUNC,                                                                                            \
                PORT_C_LINE                                                                                             \
            };                                                                                                          \
            ndebug_assert(&this_object, #expr, msg);                                                                    \
        }                                                                                                               \
    }                                                                                                                   \
    while (0u)
#else
# define NASSERT_NONAME(msg, expr)          (void)0
#endif

/**@brief       Assert macro that will always execute (no conditional).
 * @param       msg
 *              Message : enum esDebugMessageNo : enumerated debug message.
 * @param       text
 *              Text : string : a text which will be printed when this assert macro is executed.
 * @api
 */
#if (1 == CONFIG_DEBUG)
# define NASSERT_ALWAYS(text)                                                                                           \
    do                                                                                                                  \
    {                                                                                                                   \
        static const PORT_C_ROM struct ndebug_object this_object =                                                      \
        {                                                                                                               \
            &global_module_info,                                                                                        \
            PORT_C_FUNC,                                                                                                \
            PORT_C_LINE                                                                                                 \
        };                                                                                                              \
        ndebug_assert(&this_object, text, NULL);                                                                        \
    }                                                                                                                   \
    while (0u)
#else
# define NASSERT_ALWAYS(msg, text)          (void)0
#endif

/**@} *//*--------------------------------------------------------------------------------------------------------*//**
 * @name        Internal checking
 * @brief       These macros are enabled/disabled using the option @ref CONFIG_DEBUG_INTERNAL_CHECK.
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

/**@brief       Assert macro used for internal execution checking
 * @param       msg
 *              Message : enum esDebugMessageNo : enumerated debug message.
 * @param       expr
 *              Expression : C expression : condition expression which must be satisfied
 * @api
 */
#if (1 == CONFIG_DEBUG_INTERNAL)
# define NASSERT_INTERNAL(msg, expr)        NASSERT(msg, expr)
#else
# define NASSERT_INTERNAL(msg, expr)        (void)0
#endif

/**@} *//*--------------------------------------------------------------------------------------------------------*//**
 * @name        API contract validation
 * @brief       These macros are enabled/disabled using the option @ref CONFIG_DEBUG_API_VALIDATION.
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

/**@brief       Execute code to fulfill the contract
 * @param       expr
 *              Expression : C expression : expression to be executed only if contracts need to be validated.
 * @api
 */
#if (1 == CONFIG_DEBUG_API)
# define NOBLIGATION(expr)                  expr
#else
# define NOBLIGATION(expr)                  (void)0
#endif

/**@brief       Make sure the caller has fulfilled all contract preconditions
 * @param       msg
 *              Message : enum esDebugMessageNo : enumerated debug message.
 * @param       expr
 *              Expression : C expression : condition expression which must be satisfied
 * @api
 */
#if (1 == CONFIG_DEBUG_API)
# define NREQUIRE(msg, expr)                NASSERT(msg, expr)
#else
# define NREQUIRE(msg, expr)                (void)0
#endif

/**@brief       Make sure the callee has fulfilled all contract postconditions
 * @param       expr
 *              Expression : C expression : condition expression which must be satisfied
 * @api
 */
#if (1 == CONFIG_DEBUG_API)
# define NENSURE(expr)                                                                                                  \
    do                                                                                                                  \
    {                                                                                                                   \
        if ((expr) != ES_ERROR_NONE)                                                                                    \
        {                                                                                                               \
            static const PORT_C_ROM struct ndebug_object this_object =                                                  \
            {                                                                                                           \
                &global_module_info,                                                                                    \
                PORT_C_FUNC,                                                                                            \
                PORT_C_LINE                                                                                             \
            };                                                                                                          \
            ndebug_assert(&this_object, #expr, ES_API_CALL);                                                            \
        }                                                                                                               \
    }                                                                                                                   \
    while (0u)
#else
# define NENSURE(expr)                      expr
#endif

/**@brief       Make sure the callee has fulfilled all contract postconditions
 * @param       expr
 *              Expression : C expression : condition expression which must be satisfied
 * @api
 */
#if (1 == CONFIG_DEBUG_INTERNAL) && (1 == CONFIG_DEBUG_API)
# define NENSURE_INTERNAL(expr)                                                                                         \
    do                                                                                                                  \
    {                                                                                                                   \
        if ((expr) != ES_ERROR_NONE)                                                                                    \
        {                                                                                                               \
            static const PORT_C_ROM struct ndebug_object this_object =                                                  \
            {                                                                                                           \
                &global_module_info,                                                                                    \
                PORT_C_FUNC,                                                                                            \
                PORT_C_LINE                                                                                             \
            };                                                                                                          \
            ndebug_assert(&this_object, #expr, ES_API_CALL);                                                            \
        }                                                                                                               \
    }                                                                                                                   \
    while (0u)
#else
# define NENSURE_INTERNAL(expr)             expr
#endif

#define NAPI_RANGE                          "Value is out of valid range."
#define NAPI_OBJECT                         "Object is not valid."
#define NAPI_POINTER                        "Pointer has null value."
#define NAPI_USAGE                          "Object/method usage failure."
#define ES_API_CALL                         "An API method call has failed."
#define ES_ASSERT_FAILED                    "Assert failed"

/**@} *//*----------------------------------------------  C++ extern base  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*====================================================================================================  DATA TYPES  ==*/

/**@brief       Defines module information structure
 * @api
 */
struct nmodule_info
{
    const PORT_C_ROM char * const PORT_C_ROM_VAR name;                          /**<@brief Module name                */
    const PORT_C_ROM char * const PORT_C_ROM_VAR desc;                          /**<@brief Module description         */
    const PORT_C_ROM char * const PORT_C_ROM_VAR auth;                          /**<@brief Module author              */
    const PORT_C_ROM char * const PORT_C_ROM_VAR file;                          /**<@brief Module source file         */
};

/**@brief       Debug C object information structure
 * @notapi
 */
struct ndebug_object
{
    const PORT_C_ROM struct nmodule_info * const PORT_C_ROM_VAR mod;            /**<@brief Module information         */
    const PORT_C_ROM char * const PORT_C_ROM_VAR fn;                            /**<@brief Function name              */
    uint16_t                    line;                                           /**<@brief Code line reference        */
};

/**@brief       Debug report structure
 * @details     This detailed debug report prepared by ndebug_assert() function. Use this structure to present the
 *              report to the user.
 * @api
 */
struct ndebug_report
{
    const PORT_C_ROM char *     mod_name;                                       /**<@brief Module name                */
    const PORT_C_ROM char *     mod_desc;                                       /**<@brief Module description         */
    const PORT_C_ROM char *     mod_author;                                     /**<@brief Module author              */
    const PORT_C_ROM char *     mod_file;                                       /**<@brief Module source file         */
    const PORT_C_ROM char *     fn_name;                                        /**<@brief Function name              */
    const PORT_C_ROM char *     expr;                                           /**<@brief C expression               */
    const PORT_C_ROM char *     msg;                                            /**<@brief Additional text            */
    uint16_t                    line;                                           /**<@brief Source code line           */
};

/*==============================================================================================  GLOBAL VARIABLES  ==*/

/**@brief       Noname module information structure
 * @api
 */
extern PORT_C_UNUSED const PORT_C_ROM struct nmodule_info global_module_noname;

/*===========================================================================================  FUNCTION PROTOTYPES  ==*/

/**@brief       An assertion has failed
 * @param       debug_object
 *              C Object describes where the error occurred.
 * @param       expr
 *              Expression: is pointer to the string containing the expression that failed to evaluate to `TRUE`.
 * @param       msg
 *              Message: is enum esDbgMsg containing some information about the error.
 * @pre         1) `NULL != cObj`
 * @pre         2) `NULL != expr`
 * @note        1) This function is called only if @ref CONFIG_DEBUG_API is active.
 * @details     Function will prepare the information which was given by the macros and pass the information to
 *              userAssert() function for presentation.
 * @notapi
 */
PORT_C_NORETURN void ndebug_assert(
    const PORT_C_ROM struct ndebug_object * debug_object,
    const PORT_C_ROM char *     expr,
    const PORT_C_ROM char *     msg);

/**@brief       An assertion has failed. This function should inform the user about failed assertion.
 * @param       dbgReport
 *              Debug report: is pointer to the debug report created by ndebug_assert() function.
 * @pre         1) `NULL != dbgReport`
 * @note        1) This function is called only if @ref CONFIG_DEBUG is active.
 * @note        2) The function is called with interrupts disabled.
 * @details     Function will just print the information which was given by the macros.
 */
extern void userAssert(
    const struct ndebug_report * debug_report);

/*------------------------------------------------------------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*========================================================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//** @} *//*************************************************************************************
 * END of ndebug.h
 **********************************************************************************************************************/
#endif /* NDEBUG_H */
