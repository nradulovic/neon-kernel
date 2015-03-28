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
 * @brief       Memory class
 * @defgroup    mem_class Memory class
 * @brief       Memory class
 *********************************************************************//** @{ */

#ifndef NEON_MEM_CLASS_H_
#define NEON_MEM_CLASS_H_

/*=========================================================  INCLUDE FILES  ==*/

#include <stddef.h>

#include "base/port/compiler.h"
#include "base/shared/debug.h"

/*===============================================================  MACRO's  ==*/
/*------------------------------------------------------  C++ extern begin  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/

struct nmem
{
    void *                   (* vf_alloc)(struct nmem *, size_t);
    void                     (* vf_free) (struct nmem *, void *);
    void *                      base;           /**<@brief Base address       */
    size_t                      free;           /**<@brief Free bytes         */
    size_t                      size;           /**<@brief Size of memory     */
#if (CONFIG_API_VALIDATION == 1) || defined(__DOXYGEN__)
    unsigned int                signature;      /**<@brief Debug signature    */
#endif
};

typedef struct nmem nmem;

/*======================================================  GLOBAL VARIABLES  ==*/
/*===================================================  FUNCTION PROTOTYPES  ==*/



PORT_C_INLINE
void * nmem_alloc_i(
    struct nmem *               mem,
    size_t                      size)
{
    return (mem->vf_alloc(mem, size));
}




void * nmem_alloc(
    struct nmem *               mem,
    size_t                      size);



PORT_C_INLINE
void nmem_free_i(
    struct nmem *               mem,
    void *                      mem_storage)
{
    mem->vf_free(mem, mem_storage);
}



void nmem_free(
    struct nmem *               mem,
    void *                      mem_storage);



PORT_C_INLINE
size_t nmem_get_free_i(
    struct nmem *               mem)
{
    return (mem->free);
}



PORT_C_INLINE
size_t nmem_get_size_i(
    struct nmem *               mem)
{
    return (mem->size);
}

/*--------------------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of mem_class.h
 ******************************************************************************/
#endif /* NEON_MEM_CLASS_H_ */
