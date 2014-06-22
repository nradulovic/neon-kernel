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
 * @brief       Common bit/logic operations
 * @defgroup    base_bit_intf Common bit/logic operations
 * @brief       Common bit/logic operations
 *********************************************************************//** @{ */

#ifndef ES_BITOP_H_
#define ES_BITOP_H_

/*=========================================================  INCLUDE FILES  ==*/

#include "arch/cpu.h"

/*===============================================================  MACRO's  ==*/

/*------------------------------------------------------------------------*//**
 * @name        Array macros
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Determines the first dimension of an array
 * @param       array
 *              An array : type unspecified
 * @mseffect
 */
#define NARRAY_DIMENSION(array)                                               \
    (sizeof(array) / sizeof(array[0]))

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
#define ES_DIVISION_ROUND(numerator, denominator)                               \
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
#define NDIVISION_ROUNDUP(numerator, denominator)                             \
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
#define ES_ALIGN(num, align)                                                    \
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
#define ES_ALIGN_UP(num, align)                                                 \
    (((num) + (align) - 1u) & ~((align) - 1u))

/**@} *//*----------------------------------------------------------------*//**
 * @name        Log and power of 2 macros
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Da li je @c expr jednak nekom stepenu dvojke?
 * @details     Makro vraca TRUE kada je vrednost @c expr izraza jednaka nekom
 *              stepenu dvojke, inace, vraca FALSE.
 * @mseffect
 */
#define ES_IS_PWR2(num)                                                   \
    (!((num) & ((num) - 1)))

/**@brief       Calculate log2 for value @c x during the compilation
 * @mseffect
 */
#define N_LOG2_8(x)                                                        \
    ((x) <   2u ? 0u :                                                          \
     ((x) <   4u ? 1u :                                                         \
      ((x) <   8u ? 2u :                                                        \
       ((x) <  16u ? 3u :                                                       \
        ((x) <  32u ? 4u :                                                      \
         ((x) <  64u ? 5u :                                                     \
          ((x) < 128u ? 6u : 7u)))))))

/**@} *//*----------------------------------------------------------------*//**
 * @name        Simple bit operations
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Kreira masku za MSB bit koji odgovara tipu @c type
 * @param       type                    Tip podataka za koji se trazi MSB.
 * @return      Odgovarajuca binarna maska za MSB.
 */
#define ES_BIT_MASK_MSB(type)                                                   \
    (1uL << ((sizeof(type) * 8u) - 1u))

/**@brief       Postavlja MSB bit na jedan, "1".
 * @param       var                     Promenljiva kojoj se postavlja MSB bit
 *                                      na jedan.
 * @mseffect
 */
#define ES_BIT_SET_MSB(var)                                                     \
    do {                                                                        \
        var |= ES_BIT_MASK_MSB(var);                                            \
    } while (0)

/**@brief       Postavlja MSB bit na nulu, "0".
 * @param       var                     Promenljiva kojoj se postavlja MSB bit
 *                                      na nulu.
 * @mseffect
 */
#define ES_BIT_CLR_MSB(var)                                                     \
    do {                                                                        \
        var &= ~ES_BIT_MASK_MSB(var);                                           \
    } while (0)

/**@} *//*----------------------------------------------  C++ extern base  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/
/*======================================================  GLOBAL VARIABLES  ==*/
/*===================================================  FUNCTION PROTOTYPES  ==*/
/**@} *//*------------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of bitop.h
 ******************************************************************************/
#endif /* ES_BITOP_H_ */
