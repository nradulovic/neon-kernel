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
 * @author  	Nenad Radulovic
 * @brief       Main kernel interface
 * @defgroup    kern Kernel
 * @brief       Kernel overview
 * @details     Main kernel interface is divided into several sections.
 *************************************************************************************************************//** @{ */
/**@defgroup    kern_intf Interface
 * @brief       Kernel main interface
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

#if !defined(KERNEL_H__)
#define KERNEL_H__

/*=================================================================================================  INCLUDE FILES  ==*/

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "plat/compiler.h"
#include "plat/critical.h"
#include "arch/kcore.h"
#include "arch/cpu.h"
#include "arch/intr.h"
#include "arch/systimer.h"

#include "kernel/nkernel_config.h"

#include "kernel/nbitop.h"
#include "kernel/ndebug.h"
#include "kernel/nlist.h"
#include "kernel/nthread_queue.h"
#include "kernel/nsched.h"
#include "kernel/nthread.h"

/*=======================================================================================================  MACRO's  ==*/

/*------------------------------------------------------------------------*//**
 * @defgroup    kern_id Kernel identification
 * @brief       Kernel unique identification and current version number
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Identifies kernel major version number
 */
#define ES_KERN_VER_MAJOR               1ul

/**@brief       Identifies kernel minor version number
 */
#define ES_KERN_VER_MINOR               0ul

/**@brief       Identifies kernel patch level
 */
#define ES_KERN_VER_PATCH               0ul

/**@brief       Identifies the underlying kernel version number
 */
#define ES_KERN_VER                                                             \
    (((ES_KERN_VER_MAJOR) << 24) | (ES_KERN_VER_MINOR << 16) | (ES_KERN_VER_PATCH))

/**@brief       Kernel identification string
 */
#define ES_KERN_ID                      "nKernel - RT Kernel"

/**@} *//*----------------------------------------------------------------*//**
 * @defgroup    kern_thd Thread
 * @brief       Thread management services
 * @details     For more details see @ref threads.
 * @{ *//*--------------------------------------------------------------------*/



/**@} *//*----------------------------------------------  C++ extern begin  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*====================================================================================================  DATA TYPES  ==*/

/*------------------------------------------------------------------------*//**
 * @defgroup    kern_vtmr Virtual Timer
 * @brief       Virtual Timer management services
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Timer tick type
 */
#if   (2u == CFG_SYSTMR_TICK_TYPE) || defined(__DOXYGEN__)
typedef uint_fast32_t esVTmrTick;
#elif (1u== CFG_SYSTMR_TICK_TYPE)
typedef uint_fast16_t esVTmrTick;
#elif (0u == CFG_SYSTMR_TICK_TYPE)
typedef uint_fast8_t esVTmrTick;
#endif

/**@brief       Virtual Timer structure
 * @api
 */
struct esVTmr {

/**@brief       Virtual Timer linked list structure
 * @notapi
 */
    struct tmrL_ {
        struct esVTmr * q;                                                      /**< @brief Points to parent timer queue                    */
        struct esVTmr * next;                                                   /**< @brief Next thread in Virtual Timer linked list.       */
        struct esVTmr * prev;                                                   /**< @brief Previous thread in virtual timer linked list.   */
    }               tmrL;                                                       /**< @brief Virtual Timer linked List.                      */
    esVTmrTick        rtick;                                                      /**< @brief Relative tick value                             */
    void (* fn)(void *);                                                        /**< @brief Callback function pointer                       */
    void *          arg;                                                        /**< @brief Callback function argument                      */
#if   (1u == CONFIG_API_VALIDATION) || defined(__DOXYGEN__)
    natomic       signature;                                                  /**< @brief Timer structure signature                       */
#endif
};

/**@brief       Virtual Timer type
 */
typedef struct esVTmr esVTmr_T;

/** @} *//*-------------------------------------------------------------------*/

/*==============================================================================================  GLOBAL VARIABLES  ==*/
/*===========================================================================================  FUNCTION PROTOTYPES  ==*/

/*------------------------------------------------------------------------*//**
 * @defgroup    kern_general General kernel functions
 * @brief       Kernel initialization, start-up, ISR management
 * @details     There are several groups of functions:
 *              - kernel initialization and start
 *              - ISR enter and exit
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Initialize kernel internal data structures
 * @pre         1) `The kernel state == ES_KERN_INACTIVE`, see @ref states.
 * @post        1) `The kernel state == ES_KERN_INIT`.
 * @note        1) This function may be invoked only once.
 * @details     This function must be called first before any other kernel API.
 *              It initializes internal data structures that are used by other
 *              API functions.
 * @called
 * @fromapp
 * @schedno
 * @api
 */
void nsys_init(
    void);

/**@brief       Start the multi-threading
 * @pre         1) `The kernel state == ES_KERN_INIT`, see @ref states.
 * @post        1) `The kernel state == ES_KERN_RUN`
 * @post        2) The multi-threading execution will commence.
 * @note        1) Once this function is called the execution of threads will
 *                  start and this function will never return.
 * @details     This function will start multi-threading. Once the
 *              multi-threading has started the execution will never return to
 *              this function again (this function never returns).
 * @called
 * @fromapp
 * @schedyes
 * @api
 */
PORT_C_NORETURN void nsys_start(
    void);

/**@brief       Process the system timer event
 * @pre         1) `The kernel state < ES_KERN_INIT`, see @ref states.
 * @details     This function will be called only by port system timer interrupt.
 * @notapi
 */
void esKernSysTmr(
    void);

/**@brief       Enter Interrupt Service Routine
 * @pre         1) `The kernel state < ES_KERN_INIT`, see @ref states.
 * @note        1) You must call esKernIsrExitI() at the exit of ISR.
 * @note        2) You must invoke esKernIsrEnterI() and esKernIsrExitI() in pair.
 *                  In other words, for every call to esKernIsrEnterI() at the
 *                  beginning of the ISR you must have a call to esKernIsrExitI()
 *                  at the end of the ISR.
 * @details     Function will notify kernel that you are about to enter
 *              interrupt service routine (ISR). This allows kernel to keep
 *              track of interrupt nesting and then only perform rescheduling at
 *              the last nested ISR.
 * @called
 * @fromisr
 * @schedno
 * @iclass
 */
void nsys_isr_enter_i(
    void);

/**@brief       Exit Interrupt Service Routine
 * @pre         1) `The kernel state < ES_KERN_INIT`, see @ref states.
 * @note        1) You must invoke esKernIsrEnterI() and esKernIsrExitI() in pair.
 *                  In other words, for every call to esKernIsrEnterI() at the
 *                  beginning of the ISR you must have a call to esKernIsrExitI()
 *                  at the end of the ISR.
 * @note        2) Rescheduling is prevented when the scheduler is locked
 *                  (see esKernLockEnterI())
 * @details     This function is used to notify kernel that you have completed
 *              servicing an interrupt. When the last nested ISR has completed,
 *              the function will call the scheduler to determine whether a new,
 *              high-priority task, is ready to run.
 * @called
 * @fromisr
 * @schedmaybe
 * @iclass
 */
void nsys_isr_exit_i(
    void);

/**@} *//*----------------------------------------------------------------*//**
 * @addtogroup  kern_vtmr
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Add and start a new virtual timer
 * @param       vTmr
 *              Virtual Timer: is pointer to the timer ID structure, @ref esVTmr.
 * @param       tick
 *              Tick: the timer delay expressed in system ticks
 * @param       fn
 *              Function: is pointer to the callback function
 * @param       arg
 *              Argument: is pointer to the arguments of callback function
 * @pre         1) `The kernel state < ES_KERN_INACTIVE`, see @ref states.
 * @pre         2) `vTmr != NULL`
 * @pre         3) `vTmr->signature != DEF_VTMR_CONTRACT_SIGNATURE`, the timer
 *                  structure can't be initialized more than once.
 * @pre         4) `tick > 1U`
 * @pre         5) `fn != NULL`
 * @post        1) `vTmr->signature == DEF_VTMR_CONTRACT_SIGNATURE`, each
 *                  @ref esVTmr structure will have valid signature after
 *                  initialization.
 * @note        The callback function is invoked from interrupt context.
 * @called
 * @fromapp
 * @fromthd
 * @fromisr
 * @schedno
 * @iclass
 */
void esKTmrInitI(
    esVTmr_T *      vTmr,
    esVTmrTick        tick,
    void (* fn)(void *),
    void *          arg);

/**@brief       Add and start a new virtual timer
 * @param       vTmr
 *              Virtual Timer: is pointer to the timer ID structure, @ref esVTmr.
 * @param       tick
 *              Tick: the timer delay expressed in system ticks
 * @param       fn
 *              Function: is pointer to the callback function
 * @param       arg
 *              Argument: is pointer to the arguments of callback function
 * @pre         1) `The kernel state < ES_KERN_INACTIVE`, see @ref states.
 * @pre         2) `vTmr != NULL`
 * @pre         3) `vTmr->signature != DEF_VTMR_CONTRACT_SIGNATURE`, the timer
 *                  structure can't be initialized more than once.
 * @pre         4) `tick > 1U`
 * @pre         5) `fn != NULL`
 * @post        1) `vTmr->signature == DEF_VTMR_CONTRACT_SIGNATURE`, each
 *                  @ref esVTmr structure will have valid signature after
 *                  initialization.
 * @note        The callback function is invoked from interrupt context.
 * @called
 * @fromapp
 * @fromthd
 * @fromisr
 * @schedno
 * @api
 */
void esVTmrInit(
    esVTmr_T *      vTmr,
    esVTmrTick        tick,
    void (* fn)(void *),
    void *          arg);

/**@brief       Cancel and remove a virtual timer
 * @param       vTmr
 *              Timer: is pointer to the timer ID structure, @ref esVTmr.
 * @pre         1) `The kernel state < ES_KERN_INACTIVE`, see @ref states.
 * @pre         2) `vTmr != NULL`
 * @pre         3) `vTmr->signature == DEF_VTMR_CONTRACT_SIGNATURE`, the pointer
 *                  must point to a valid @ref esVTmr structure.
 * @post        1) `vTmr->signature = ~DEF_VTMR_CONTRACT_SIGNATURE`, each
 *                  @ref esVTmr structure will have invalid signature after
 *                  termination.
 * @called
 * @fromapp
 * @fromthd
 * @fromisr
 * @schedno
 * @iclass
 */
void esVTmrTermI(
    esVTmr_T *       vTmr);

/**@brief       Cancel and remove a virtual timer
 * @param       vTmr
 *              Timer: is pointer to the timer ID structure, @ref esVTmr.
 * @pre         1) `The kernel state < ES_KERN_INACTIVE`, see @ref states.
 * @pre         2) `vTmr != NULL`
 * @pre         3) `vTmr->signature == DEF_VTMR_CONTRACT_SIGNATURE`, the pointer
 *                  must point to a valid @ref esVTmr structure.
 * @post        1) `vTmr->signature = ~DEF_VTMR_CONTRACT_SIGNATURE`, each
 *                  @ref esVTmr structure will have invalid signature after
 *                  termination.
 * @called
 * @fromapp
 * @fromthd
 * @fromisr
 * @schedno
 * @api
 */
void esVTmrTerm(
    esVTmr_T *       vTmr);

/**@brief       Delay for specified amount of ticks
 * @param       tick
 *              Tick: number of system ticks to delay.
 * @details     This function will create a virtual timer with count down time
 *              specified in argument `tick` and put the calling thread into
 *              `sleep` state. When timeout expires the thread will be placed
 *              back into `ready` state.
 * @note        The sleeping thread can not be safely awaken until the
 *              specified time does not expire.
 * @pre         1) `tick > 1u`
 * @called
 * @fromapp
 * @fromthd
 * @fromisr
 * @schedyes
 * @api
 */
void esVTmrDelay(
    esVTmrTick        tick);

/**@} *//*----------------------------------------------------------------*//**
 * @defgroup    kern_time Kernel time
 * @brief       Kernel time management
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Get the current tick value
 * @return      Current tick value
 * @pre         1) `The kernel state < ES_KERN_INACTIVE`, see @ref states.
 * @called
 * @fromapp
 * @fromthd
 * @fromisr
 * @schedno
 * @api
 */
esVTmrTick esSysTmrTickGet(
    void);

/**@} *//*----------------------------------------------------------------*//**
 * @defgroup    kern_hook Kernel hook functions
 * @brief       User defined hook (callback) function prototypes
 * @note        1) The definition of this functions must be written by the user.
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       System timer hook function, called from system system timer ISR
 *              function before the kernel functions.
 * @note        1) This function is called only if
 *              @ref CFG_HOOK_PRE_SYSTMR_EVENT is active.
 * @details     This function is called whenever a system event is generated.
 */
extern void userPreSysTmr(
    void);

/**@brief       Kernel initialization hook function, called from esKernInit()
 *              function before kernel initialization.
 * @note        1) This function is called only if @ref CFG_HOOK_PRE_KERN_INIT
 *              is active.
 * @details     This function is called before the kernel initialization.
 */
extern void userPreKernInit(
    void);

/**@brief       Kernel initialization hook function, called from esKernInit()
 *              function after kernel initialization.
 * @note        1) This function is called only if
 *              @ref CFG_HOOK_POST_KERN_INIT is active.
 * @details     This function is called after the kernel initialization.
 */
extern void userPostKernInit(
    void);

/**@brief       Kernel start hook function, called from esKernStart() function.
 * @note        1) This function is called only if @ref CFG_HOOK_PRE_KERN_START
 *              is active.
 * @details     This function is called before kernel start.
 */
extern void userPreKernStart(
    void);

/**@brief       Thread initialization end hook function, called from esThdInit()
 *              function.
 * @param       thd
 *              Thread: pointer to thread Id structure that has just been
 *              initialized.
 * @note        1) This function is called only if @ref CFG_HOOK_POST_THD_INIT
 *              is active.
 * @details     This function is called after the thread initialization.
 */
extern void userPostThdInit(
    nthread *       thd);

/**@brief       Thread terminate hook function, called from esThdTerm() or when
 *              a thread terminates itself.
 * @note        1) This function is called only if @ref CFG_HOOK_PRE_THD_TERM is
 *              active.
 */
extern void userPreThdTerm(
    void);

/**@brief       Pre Idle hook function, called from idle thread, just before
 *              entering idle period.
 * @note        1) This function is called only if @ref CFG_HOOK_PRE_IDLE and
 *              @ref CFG_SCHED_POWER_SAVE are active.
 * @note        2) This function is called with interrupts and scheduler locked.
 */
extern void userPreIdle(
    void);

/**@brief       Post idle hook function, called from idle thread, just after
 *              exiting idle period.
 * @note        1) This function is called only if @ref CFG_HOOK_POST_IDLE and
 *              @ref CFG_SCHED_POWER_SAVE are active.
 * @note        2) This function is called with scheduler locked.
 */
extern void userPostIdle(
    void);

/**@brief       Kernel context switch hook function, called from esSchedYieldI()
 *              and esSchedYieldIsrI() functions just before context switch.
 * @param       oldThd
 *              Pointer to the thread being switched out.
 * @param       newThd
 *              Pointer to the thread being switched in.
 * @note        1) This function is called only if @ref CFG_HOOK_PRE_CTX_SW is
 *              active.
 * @details     This function is called at each context switch.
 */
extern void userPreCtxSw(
    nthread *       oldThd,
    nthread *       newThd);

/**@} *//*------------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*========================================================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//**@} *//**@} *//***************************************************************************************
 * END of kernel.h
 **********************************************************************************************************************/
#endif /* KERNEL_H__ */
