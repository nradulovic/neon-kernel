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
 * @author  	Nenad Radulovic
 * @brief       Common bit/logic operations
 * @defgroup    bitop_intf Common bit/logic operations
 * @brief       Common bit/logic operations
 *********************************************************************//** @{ */

#ifndef NBITOP_H_
#define NBITOP_H_

/*=========================================================  INCLUDE FILES  ==*/
/*===============================================================  MACRO's  ==*/

/*------------------------------------------------------------------------*//**
 * @name        Array macros
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Determines the first dimension of an array
 * @param       array
 *              An array : type unspecified
 * @mseffect
 */
#define NARRAY_DIMENSION(array)             (sizeof(array) / sizeof(array[0]))

/**@} *//*----------------------------------------------------------------*//**
 * @name        Integer division
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Round a division
 * @param       numerator
 * @param       denominator
 * @note        It is desirable that denominator is a constant expression,
 *              otherwise the compiler will generate two division operations.
 * @mseffect
 * @par         Example 1:
 *
 *              numerator   = 28
 *              denominator = 5
 *
 *              Integer division: 28 / 5 = 5
 *              Float division  : 28 / 5 = 5.6
 *              Rounded division: 28 / 5 = 6
 *
 * @par         Example 2:
 *
 *              numerator   = 27
 *              denominator = 5
 *
 *              Integer division: 27 / 5 = 5
 *              Float division  : 27 / 5 = 5.4
 *              Rounded division: 27 / 5 = 5
 */
#define NDIVISION_ROUND(numerator, denominator)                                 \
    (((numerator) + ((denominator) / 2u)) / (denominator))

/**@brief       Round up a division
 * @param       numerator
 * @param       denominator
 * @note        It is desirable that denominator is a constant expression,
 *              otherwise the compiler will generate one subtraction and one
 *              division operation.
 * @mseffect
 * @par         Example 1:
 *
 *              numerator   = 28
 *              denominator = 5
 *
 *              Integer division   : 28 / 5 = 5
 *              Float division     : 28 / 5 = 5.6
 *              Rounded up division: 28 / 5 = 6
 *
 * @par         Example 2:
 *
 *              numerator   = 27
 *              denominator = 5
 *
 *              Integer division   : 27 / 5 = 5
 *              Float division     : 27 / 5 = 5.4
 *              Rounded up division: 27 / 5 = 6
 */
#define NDIVISION_ROUNDUP(numerator, denominator)                               \
    (((numerator) + (denominator) - 1u) / (denominator))

/**@} *//*----------------------------------------------------------------*//**
 * @name        Data alignment
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Vrsi poravnjanje @a num promenjive sa granicama specificarane
 *              u @a align
 * @param       num                     Promenjiva koju treba poravnati,
 * @param       align                   granica poravnanja.
 * @mseffect
 * @par         Example 1:
 *
 *              num  : 10010101 = 149
 *              align: 00000100 = 4
 *              Result is 148.
 */
#define NALIGN(num, align)                                                      \
    ((num) & ~((align) - 1u))

/**@brief       Vrsi poravnjanje @a num promenjive sa granicama specificarane
 *              u @a align
 * @param       num                     Promenjiva koju treba poravnati,
 * @param       align                   granica poravnanja.
 * @mseffect
 * @par         Example 1:
 *
 *              num  : 10010101 = 149
 *              align: 00000100 = 4
 *              Result is 152.
 */
#define NALIGN_UP(num, align)                                                   \
    (((num) + (align) - 1u) & ~((align) - 1u))

/**@} *//*----------------------------------------------------------------*//**
 * @name        Logarithm of base 2
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Calculate log2 for value @c x during the compilation
 * @mseffect
 */
#define NLOG2_8(x)                                                              \
    ((x) <   2u ? 0u :                                                          \
     ((x) <   4u ? 1u :                                                         \
      ((x) <   8u ? 2u :                                                        \
       ((x) <  16u ? 3u :                                                       \
        ((x) <  32u ? 4u :                                                      \
         ((x) <  64u ? 5u :                                                     \
          ((x) < 128u ? 6u : 7u)))))))

/**@} *//*-----------------------------------------------  C++ extern base  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/
/*======================================================  GLOBAL VARIABLES  ==*/
/*===================================================  FUNCTION PROTOTYPES  ==*/
/*--------------------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of nbitop.h
 ******************************************************************************/
#endif /* NBITOP_H_ */
