/*
 * This file is part of Neon.
 *
 * Copyright (C) 2010 - 2015 Nenad Radulovic
 *
 * Neon is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Neon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Neon.  If not, see <http://www.gnu.org/licenses/>.
 *
 * web site:    http://github.com/nradulovic
 * e-mail  :    nenad.b.radulovic@gmail.com
 *//***********************************************************************//**
 * @file
 * @author      Nenad Radulovic
 * @brief       Static Memory management
 * @defgroup    mem_static Static Memory management
 * @brief       Static Memory Management
 *********************************************************************//** @{ */

#ifndef NEON_MEM_STATIC_H_
#define NEON_MEM_STATIC_H_

/*=========================================================  INCLUDE FILES  ==*/

#include <stddef.h>

#include "mm/mem.h"

/*===============================================================  MACRO's  ==*/

/*-------------------------------------------------------  C++ extern base  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/

/**@brief       Static memory instance handle structure
 * @details     This structure holds information about static memory instance.
 * @api
 */
struct nstatic
{
    struct nmem                 mem_class;
};

/**@brief       Static memory instance handle type
 * @api
 */
typedef struct nstatic nstatic;

/*======================================================  GLOBAL VARIABLES  ==*/
/*===================================================  FUNCTION PROTOTYPES  ==*/


/**@brief       Initializes static memory instance
 * @param       static_mem
 *              Pointer to handle type variable, see @ref nstatic.
 * @param       storage
 *              Storage memory reserved for static memory manager.
 * @param       size
 *              Size of reserved memory expresses in bytes.
 * @details     This function shall be called before any other static memory
 *              management function.
 * @api
 */
void nstatic_init(
    struct nstatic *            static_mem,
    void *                      storage,
    size_t                      size);



/**@brief       Allocates static memory of get_size @c get_size
 * @param       static_mem
 *              Pointer to static memory instance, see @ref nstatic.
 * @param       size
 *              The size of requested memory in bytes.
 * @return      Pointer to free memory of requested get_size.
 * @iclass
 */
void * nstatic_alloc_i(
    struct nstatic *            static_mem,
    size_t                      size);



/**@brief       Allocates static memory of get_size @c get_size
 * @param       static_mem
 *              Pointer to static memory instance, see @ref nstatic.
 * @param       size
 *              The size of requested memory in bytes.
 * @return      Pointer to free memory of requested get_size.
 * @api
 */
void * nstatic_alloc(
    struct nstatic *            static_mem,
    size_t                      size);

/*--------------------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of static.h
 ******************************************************************************/
#endif /* NEON_MEM_STATIC_H_ */
