/*
 * This file is part of eSolid-Kernel
 *
 * Copyright (C) 2013 - Nenad Radulovic
 *
 * eSolid-Kernel is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * eSolid-Kernel is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with eSolid-Kernel; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 *
 * web site:    http://blueskynet.dyndns-server.com
 * e-mail  :    blueskyniss@gmail.com
 *//***********************************************************************//**
 * @file
 * @author  	Nenad Radulovic
 * @brief       Interface of CPU port - Template
 * @addtogroup  template_cpu_intf
 * @details     Since this header file is included with the API of the kernel a
 *              few naming conventions are defined in order to avoid name
 *              clashing with the names of objects from libraries included by
 *              application code.
 *
 * @par         1) Macro naming conventions
 *              For macro naming try to follow these rules:
 *              - All standard PORT API macro names are prefixed with:
 *              @b @c PORT_.
 *              - All other macros which are specific to the port used are
 *              prefixed with: @b @c CPU_.
 *
 * @par         2) Type declaration naming conventions
 *              For type declaration naming try to follow these rules:
 *              - All type declaration names are prefixed with: @b @c cpu.
 *
 * @par         3) Global variable naming conventions
 *              For global variable naming try to follow these rules:
 *              - All global variable names are prefixed with: @b @c cpu.
 *
 * @par         4) Funcion naming convetions
 *              For functions naming try to follow these rules:
 *              - All function names are prefixed with: <b><code>port</code></b>
 *              and postfixed with: @b @c _ (underscore).
 *              - All other functions which are specific to the port used are
 *              prefixed with: <b><code>cpu</code></b> and postfixed with:
 *              @b @c _ (underscore).
 *              - The @c exception to above two rules are the names of functions
 *              used for Interrupt Service Routines. They can have any name
 *              required by port.
 *
 *********************************************************************//** @{ */

#ifndef CPU_H_
#define CPU_H_

/*=========================================================  INCLUDE FILES  ==*/
#include "cpu_cfg.h"

/*===============================================================  MACRO's  ==*/

/*------------------------------------------------------------------------*//**
 * @name        Port constants
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       This macro specifies the bit width of CPU data registers
 */
#define PORT_DATA_WIDTH_VAL             8U

/**@brief       This macro specifies the minimal size of the thread stack
 * @details     Generally minimal stack size is equal to the size of context
 *              structure
 */
#define PORT_STCK_MINSIZE_VAL                                                   \
    (sizeof(struct portCtx) / sizeof(portReg_T))

/**@brief       System timer reload value for one tick
 * @details     This is a calculated value for one system tick period
 */
#define PORT_SYSTMR_ONE_TICK_VAL                                                  \
    (CFG_SYSTMR_CLOCK_FREQUENCY / CFG_SYSTMR_EVENT_FREQUENCY)

/**@brief       System timer maximum value
 * @details     This macro specifies maximum value that can be reloaded into
 *              system timer counter. For example, if the system timer is a
 *              8-bit counter than this macro would have the value of 0xFFU.
 */
#define PORT_SYSTMR_MAX_VAL             0xFFU

/**@brief       Maximum number of ticks the system timer can accept
 */
#define PORT_SYSTMR_MAX_TICKS_VAL                                               \
    (PORT_SYSTMR_MAX_VAL / PORT_SYSTMR_RELOAD_VAL)

/**@brief       Kernel Virtual Timer Thread stack size
 */
#define PORT_KVTMR_STCK_SIZE            40U

/**@brief       Kernel Idle Thread stack size
 */
#define PORT_KIDLE_STCK_SIZE            40U

/** @} *//*---------------------------------------------------------------*//**
 * @name        Interrupt management
 * @details     PORT_ISR_... macros are used by esKernIsrEnter() and
 *              esKernIsrExit() functions. They are used to keep the current
 *              level of ISR nesting. Scheduler should be invoked only from the
 *              last ISR that is executing.
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Disable all interrupt sources
 */
#define PORT_INT_DISABLE()

/**@brief       Enter ISR. Increment gPortIsrNesting_ variable to keep track of
 *              ISR nesting.
 * @details     Variable gPortIsrNesting_ is needed only if the port does not
 *              support any other method of detecting when the last ISR is
 *              executing.
 */
#define PORT_ISR_ENTER()                                                        \
    do {                                                                        \
        gPortIsrNesting_++;                                                     \
        esKernIsrPrologueI();                                                   \
    } while (0U)

/**@brief       Exit ISR. Decrement gPortIsrNesting_ variable to keep track of
 *              ISR nesting.
 * @details     Variable gPortIsrNesting_ is needed only if the port does not
 *              support any other method of detecting when the last ISR is
 *              executing.
 */
#define PORT_ISR_EXIT()                                                         \
    do {                                                                        \
        gPortIsrNesting_--;                                                     \
        esKernIsrEpilogueI();                                                   \
    } while (0U)

/**@brief       If isrNesting variable is zero then the last ISR is executing
 *              and scheduler should be invoked
 * @return      Is the currently executed ISR the last one?
 *  @retval     TRUE - this is last ISR
 *  @retval     FALSE - this is not the last ISR
 */
#define PORT_ISR_IS_LAST()              (0U == gPortIsrNesting ? TRUE : FALSE)

/** @} *//*---------------------------------------------------------------*//**
 * @name        Critical section management
 * @brief       Disable/enable interrupts by preserving the status of interrupts.
 * @details     Generally speaking these macros would store the status of the
 *              interrupt disable flag in the local variable declared by
 *              @ref PORT_CRITICAL_DECL and then disable interrupts. Local
 *              variable is allocated in all of eSolid RTOS functions that need
 *              to disable interrupts.  Macros would restore the interrupt
 *              status by copying back the allocated variable into the CPU's
 *              status register.
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Declare the interrupt status variable
 * @details     This variable is used to store the current state of enabled ISRs.
 */
#define PORT_CRITICAL_DECL()            portReg_T intStatus_

/**@brief       Enter critical section
 */
#define PORT_CRITICAL_ENTER()

/**@brief       Exit critical section
 */
#define PORT_CRITICAL_EXIT()

/** @} *//*---------------------------------------------------------------*//**
 * @name        Scheduler support
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Find last set bit in a word
 * @details     This function is used by the scheduler to efficiently determine
 *              the highest priority of thread ready for execution. For
 *              algorithm details see:
 *              http://en.wikipedia.org/wiki/Find_first_set.
 * @return      The position of the last set bit in a word
 */
#define PORT_FIND_LAST_SET(val)         portFindLastSet_(val)

/**@brief       Helper macro: calculate 2^pwr expression
 * @details     Some ports may want to use look up tables instead of shifting
 *              operation
 */
#define PORT_PWR2(pwr)                  (1U << (pwr))

/**@brief       Initialize system timer and associated interrupt
 * @details     This macro will only initialize system timer and associated
 *              interrupt. The macro is called from esKernStart() function.
 *              Responsibility:
 *              - initialize system timer
 *              - initialize system timer interrupt
 * @note        This macro MUST NOT enable system timer events. System timer
 *              events are enabled/disabled by PORT_SYSTMR_ISR_ENABLE() and
 *              PORT_SYSTMR_ISR_DISABLE() macros.
 */
#define PORT_SYSTMR_INIT()

/**@brief       Stop the timer if it is running and disable associated interrupt.
 * @details     Responsibility:
 *              - disable system timer interrupt
 *              - stop and disable system timer
 */
#define PORT_SYSTMR_TERM()

/**@brief       Get system timer reload value
 */
#define PORT_SYSTMR_GET_RVAL()

/**@brief       Get system timer current value
 */
#define PORT_SYSTMR_GET_CVAL()

/**@brief       Reload the system timer with specified number
 * @details     Responsibility:
 *              - stop the system timer
 *              - reload the system timer
 *              - start the system timer
 */
#define PORT_SYSTMR_RLD(val)

/**@brief       Enable the system timer
 * @details     Responsibility:
 *              - enable (run) the system timer counter
 */
#define PORT_SYSTMR_ENABLE()

/**@brief       Disable the system timer
 * @details     Responsibility:
 *              - disable (stop) the system timer counter
 */
#define PORT_SYSTMR_DISABLE()

/**@brief       Enable the system timer interrupt
 * @details     Responsibility:
 *              - allow system timer interrupt to occur
 */
#define PORT_SYSTMR_ISR_ENABLE()

/**@brief       Disable the system timer interrupt
 * @details     Responsibility:
 *              - disallow system timer interrupt to occur
 */
#define PORT_SYSTMR_ISR_DISABLE()

/** @} *//*---------------------------------------------------------------*//**
 * @name        Dispatcher context switching
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Initialize the thread context
 * @param       [inout] stck
 *              Pointer to the allocated thread stck. The pointer points to the
 *              beginning of the memory as defined per C language. It's up to
 *              port function to adjust the pointer according to the stck type:
 *              full descending or full ascending one.
 * @param       stackSize
 *              The size of allocated stck in bytes.
 * @param       [in] thread
 *              Pointer to the thread function.
 * @param       [in] arg
 *              Argument that will be passed to thread function at the starting
 *              of execution.
 * @return      The new top of stck after thread context initialization.
 */
#define PORT_CTX_INIT(stck, stackSize, thread, arg)

/**@brief       Do the context switch - invoked from API level
 */
#define PORT_CTX_SW()

/**@brief       Do the context switch - invoked from ISR level
 */
#define PORT_CTX_SW_ISR()

/**@brief       Start the first thread
 */
#define PORT_THD_START()

/** @} *//*---------------------------------------------------------------*//**
 * @name        General port macros
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Calculate the stack size
 * @details     This macro is used when specifying the size of thread stack.
 *              Responsibility:
 *              - add to @p size the minimal stack size specified by
 *              @ref PORT_STCK_MINSIZE_VAL.
 *              - if it is needed by the port make sure the alignment is correct.
 */
#define PORT_STCK_SIZE(size)                                                    \
    ((((size + PORT_STCK_MINSIZE_VAL) + (sizeof(struct portStck) /                  \
    sizeof(portReg_T))) - 1U) / (sizeof(struct portStck)/sizeof(portReg_T)))

/**@brief       Exit critical section and enter sleep state
 */
#define PORT_CRITICAL_EXIT_SLEEP()                                              \
    portIntSetSleepEnter_(intStatus_)

/**@brief       Early port initialization
 * @details     This macro will be called at early initialization stage from
 *              esKernInit() function. It is called before any kernel data
 *              initialization. Usually this macro would be used to setup
 *              memory space, fill the memory with debug value or something
 *              similar.
 */
#define PORT_INIT_EARLY()

/**@brief       Port initialization
 * @details     This macro will be called after kernel data structure
 *              initialization from esKernInit() function.
 */
#define PORT_INIT()

/**@brief       Late port initialization
 * @details     This macro will be called just a moment before the multitasking
 *              is started. The macro is called from esKernStart() function.
 */
#define PORT_INIT_LATE()

/** @} *//*---------------------------------------------  C++ extern begin  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/

/**@brief       Data type which corresponds to the general purpose register
 */
typedef uint8_t portReg_T;

/**@brief       Stack structure used for stack declaration in order to force the
 *              alignment
 */
struct portStck {
    portReg_T       reg;                                                        /**< @brief A structure field representing stack data       */
};                                                                              /**< @brief Alignment of stack structure                    */

typedef struct portStck portStck_T;                                             /**< @brief Stack type                                      */

/**@brief       Port context structure
 */
struct portCtx {
    portReg_T       r0;                                                         /**< @brief Data pushed on stack during context switching   */
};

/*======================================================  GLOBAL VARIABLES  ==*/

/**@brief       Variable to keep track of ISR nesting.
 */
extern portReg_T gPortIsrNesting_;

/**@brief       Look up table for: 2^n expression
 * @details     This look up table can be used to accelerate the Logical Shift
 *              Left operations which are needed to set bits inside the priority
 *              bit map. In plain C this operation would be written as:
 *              <code>(1U << n)</code>, but in many 8-bit CPUs this operation
 *              can be lengthy. If there is a need for faster operation than
 *              this table can be used instead of the mentioned C code.
 *
 *              To use the look up table change @ref PORT_PWR2 macro
 *              implementation from: <code>(1U << (pwr))</code> to
 *              <code>pwr2LKP[pwr]</code>
 */
extern const PORT_C_ROM portReg_T pwr2LKP [PORT_DATA_WIDTH_VAL];

/*===================================================  FUNCTION PROTOTYPES  ==*/

/*------------------------------------------------------------------------*//**
 * @name        Scheduler support
 * @note        These functions are extensively used by the scheduler and
 *              therefore they should be optimized for the architecture being
 *              used.
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Find last set bit in a word
 * @param       val
 *              Value which needs to be evaluated
 * @details     This function is used by the scheduler to efficiently determine
 *              the highest priority of thread ready for execution. For
 *              algorithm details see:
 *              http://en.wikipedia.org/wiki/Find_first_set.
 * @return      The position of the last set bit in a word
 */
uint_fast8_t portFindLastSet_(
    portReg_T       val);

/** @} *//*---------------------------------------------------------------*//**
 * @name        Dispatcher context switching
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Initialize the thread context
 * @param       stck
 *              Pointer to the allocated thread stck. The pointer points to the
 *              beginning of the memory as defined per C language. It's up to
 *              port function to adjust the pointer according to the stck type:
 *              full descending or full ascending one.
 * @param       stckSize
 *              The size of allocated stck in bytes.
 * @param       fn
 *              Pointer to the thread function.
 * @param       arg
 *              Argument that will be passed to thread function at the starting
 *              of execution.
 * @return      The new top of stck after thread context initialization.
 */
void * portCtxInit_(
    void *          stck,
    size_t          stckSize,
    void (* fn)(void *),
    void *          arg);

/** @} *//*---------------------------------------------  C++ extern begin  --*/
#ifdef __cplusplus
}
#endif

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of cpu.h
 ******************************************************************************/
#endif /* CPU_H_ */
