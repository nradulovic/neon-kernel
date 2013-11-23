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
 * @brief       General purpose bit operations
 * @addtogroup  kern_bit_intf
 *********************************************************************//** @{ */

#if !defined(BITOP_H__)
#define BITOP_H__

/*=========================================================  INCLUDE FILES  ==*/

#include "arch/cpu.h"

/*===============================================================  MACRO's  ==*/


/*------------------------------------------------------------------------*//**
 * @name        General Purpose macros
 * @details     These macros are used for:
 *              - string concatenation
 *              - simple arithmetic operations
 *              - address and bitwise operations
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Determines the first dimension of an array
 * @param       array
 *              An array : type unspecified
 * @mseffect
 */
#define ES_BIT_DIMENSION(array)                                                 \
    (sizeof(array) / sizeof(array[0]))

/**@brief       Vrsi deljenje dva broja i zaokruzuje rezultat.
 * @param       numerator
 *              deljenik
 * @param       denominator
 *              delitelj
 * @note        Pozeljno je da @a expr2 bude konstanta, inace, generisace se dva
 *              deljenja.
 * @mseffect
 * @par         Primer:
 *
 * @par         Izlaz:
 *
 *              @code
 *              Deljenje 28 / 5 = 5
 *              Zaokruzeno: 6
 *              @endcode
 */
#define ES_BIT_DIV_ROUND(numerator, denominator)                                \
    (((numerator) + ((denominator) / 2U)) / (denominator))

/**@brief       Vrsi deljenje dva broja sa zaokruzivanjem broja navise.
 * @param       numerator
 *              deljenik
 * @param       denominator
 *              delitelj
 * @note        Pozeljno je da @a denominator bude konstanta, inace, generisace se
 *              jedno oduzimanje i jedno deljenje.
 * @mseffect
 * @par         Primer:
 * @par         Izlaz:
 *
 *              @code
 *              Deljenje 27 / 5 = 5
 *              Zaokruzeno: 6
 *              @endcode
 */
#define ES_BIT_DIV_ROUNDUP(numerator, denominator)                              \
    (((numerator) + (denominator) - 1u) / (denominator))

/**@brief       Vraca vecu vrednost od ponudjenih vrednosti @a expr1 i @a expr2
 */
#define ES_BIT_MAX(expr1, expr2)                                                \
    ((expr1 > expr2) ? expr1 : expr2))

/**@brief       Vraca manju vrednost od ponudjenih vrednosti @a expr1 i @a expr2
 */
#define ES_BIT_MIN(expr1, expr2)                                                \
    ((expr1 < expr2) ? expr1 : expr2))

/**@brief       Vrsi poravnjanje @a num promenjive sa granicama specificarane
 *              u @a align
 * @param       num                     Promenjiva koju treba poravnati,
 * @param       align                   granica poravnanja.
 * @details     Primer:
 *              @a expr1 ima binarnu vrednost: 10010101 = 149
 *              @a expr2 ima binarnu vrednost: 00000100 = 4
 *              Dobija se 152.
 * @mseffect
 */
#define ES_BIT_ALIGN(num, align)                                                \
    ((num) & ~((align) - 1u))

/**@brief       Vrsi poravnjanje @a num promenjive sa granicama specificarane
 *              u @a align
 * @param       num                     Promenjiva koju treba poravnati,
 * @param       align                   granica poravnanja.
 * @details     Primer:
 *              @a expr1 ima binarnu vrednost: 10010101 = 149
 *              @a expr2 ima binarnu vrednost: 00000100 = 4
 *              Dobija se 152.
 * @mseffect
 */
#define ES_BIT_ALIGN_UP(num, align)                                             \
    (((num) + (align) - 1u) & ~((align) - 1u))

/**@brief       Find last set bit in a word
 * @param       val
 *              Value : portReg_T, value which needs to be evaluated
 * @return      The position of the last set bit in a value
 * @details     This function is used by the scheduler to efficiently determine
 *              the highest priority of thread ready for execution. For similar
 *              algorithm details see:
 *              http://en.wikipedia.org/wiki/Find_first_set.
 */
#define ES_BIT_FIND_LAST_SET(val)       PORT_BIT_FIND_LAST_SET(val)

/**@brief       Helper macro: calculate 2^pwr expression
 * @param       pwr
 *              Power : portReg_T, value which will be used in calculation
 * @details     Some ports may want to use look up tables instead of shifting
 *              operation
 */
#define ES_BIT_BIT_PWR2(pwr)            PORT_BIT_PWR2(pwr)

/**@brief       Da li je @c expr jednak nekom stepenu dvojke?
 * @details     Makro vraca TRUE kada je vrednost @c expr izraza jednaka nekom
 *              stepenu dvojke, inace, vraca FALSE.
 * @mseffect
 */
#define ES_BIT_IS_POW2(num)                                                     \
    (!((num) & ((num) - 1)))

/**@brief       Calculate log2 for value @c x during the compilation
 * @mseffect
 */
#define ES_BIT_UINT8_LOG2(x)                                                    \
    ((x) <   2u ? 0u :                                                          \
     ((x) <   4u ? 1u :                                                         \
      ((x) <   8u ? 2u :                                                        \
       ((x) <  16u ? 3u :                                                       \
        ((x) <  32u ? 4u :                                                      \
         ((x) <  64u ? 5u :                                                     \
          ((x) < 128u ? 6u : 7u)))))))

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

/**@} *//*----------------------------------------------  C++ extern begin  --*/
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
 * END of bitop.h
 ******************************************************************************/
#endif /* BITOP_H__ */
