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
 *              - All type declaration names are prefixed with: @b @c port.
 *
 * @par         3) Global variable naming conventions
 *              For global variable naming try to follow these rules:
 *              - All global variable names are prefixed with: @b @c Port.
 *
 * @par         4) Function naming conventions
 *              For functions naming try to follow these rules:
 *              - All standard PORT API function names are prefixed with:
 *              <b><code>port</code></b>.
 *              - All other functions which are specific to the port used are
 *              prefixed with: <b><code>cpu</code></b>
 *              - All inline functions are additionally postfixed with: @b @c _
 *              (underscore).
 *              - The @c exception to above two rules are the names of functions
 *              used for Interrupt Service Routines. They can have any name
 *              required by the port.
 *
 *********************************************************************//** @{ */

#if !defined(CPU_H__)
#define CPU_H__

/*=========================================================  INCLUDE FILES  ==*/

#include "arch/compiler.h"
#include "arch/cpu_cfg.h"
#include "kernel_cfg.h"

/*===============================================================  MACRO's  ==*/

/*------------------------------------------------------------------------*//**
 * @name        Port constants
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       This macro specifies the bit width of atomic data type portReg_T
 * @details     To avoid uncertainty about interrupting access to a variable,
 *              you can use a particular data type for which access is always
 *              atomic: @ref portReg_T.
 */
#define PORT_DEF_DATA_WIDTH             8u

/**@brief       Defines required data alignment in bytes
 */
#define PORT_DEF_DATA_ALIGNMENT         1u

/**@brief       Defines maximum interrupt priority which can use kernel services
 */
#define PORT_DEF_MAX_ISR_PRIO           (PORT_CFG_MAX_ISR_PRIO)

/**@brief       This macro specifies the minimal size of the thread stack
 * @details     Generally minimal stack size is equal to the size of context
 *              structure
 */
#define PORT_DEF_STCK_MINSIZE                                                   \
    (sizeof(struct portCtx) / sizeof(portReg_T))

/**@} *//*----------------------------------------------------------------*//**
 * @name        System timer constants
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       System timer reload value for one tick
 * @details     This is a calculated value for one system tick period
 */
#define PORT_DEF_SYSTMR_ONE_TICK                                                \
    (CFG_SYSTMR_CLOCK_FREQUENCY / CFG_SYSTMR_EVENT_FREQUENCY)

/**@brief       Maximum number of ticks without overflowing the system timer
 * @details     This macro expects that CPU_DEF_SYSTMR_MAX_VAL is set in CPU
 *              profile
 */
#define PORT_DEF_SYSTMR_MAX_TICKS                                               \
    (CPU_DEF_SYSTMR_MAX_VAL / PORT_DEF_SYSTMR_ONE_TICK)

/**@brief       Threshold system timer value for new tick
 */
#define PORT_DEF_SYSTMR_WAKEUP_TH_VAL   600u

/**@} *//*----------------------------------------------------------------*//**
 * @name        Kernel threads port dependent settings
 * @brief       Kernel uses several threads for system management. This section
 *              defines port dependent settings for the threads.
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Kernel Virtual Timer Thread stack size
 * @todo        This value needs tweaking
 */
#define PORT_DEF_KVTMR_STCK_SIZE        40u

/**@brief       Kernel Idle Thread stack size
 * @todo        This value needs tweaking
 */
#define PORT_DEF_KIDLE_STCK_SIZE        40u

/**@} *//*----------------------------------------------------------------*//**
 * @name        Interrupt management
 * @details     PORT_ISR_... macros are used by esKernIsrEnter() and
 *              esKernIsrExit() functions. They are used to keep the current
 *              level of ISR nesting. Scheduler should be invoked only from the
 *              last ISR that is executing.
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Global Enable interrupt sources
 */
#define PORT_INT_ENABLE()

/**@brief       Global Disable all interrupt sources
 */
#define PORT_INT_DISABLE()

/**@brief       Set interrupt priority mask
 * @param       prio
 *              Priority : @ref portReg_T type value for interrupt priority mask
 */
#define PORT_INT_PRIO_SET(prio)

/**@brief       Get current interrupt priority mask
 * @param       prio
 *              Priority : pointer to variable of type @ref portReg_T which will
 *              hold the priority value.
 */
#define PORT_INT_PRIO_GET(prio)

/**@brief       Get current and set new interrupt priority mask
 * @param       oldPrio
 *              Old priority : pointer to variable of type @ref portReg_T which
 *              will hold old priority value.
 * @param       newPrio : @ref portReg_T type value
 *              New priority mask value
 */
#define PORT_INT_PRIO_REPLACE(oldPrio, newPrio)

/**@brief       Enter ISR. Increment PortIsrNesting variable to keep track of
 *              ISR nesting.
 * @details     Variable PortIsrNesting is needed only if the port does not
 *              support any other method of detecting when the last ISR is
 *              executing.
 */
#define PORT_ISR_ENTER()                                                        \
    do {                                                                        \
        PortIsrNesting++;                                                       \
        esKernIsrEnterI();                                                      \
    } while (0u)

/**@brief       Exit ISR. Decrement PortIsrNesting variable to keep track of
 *              ISR nesting.
 * @details     Variable PortIsrNesting is needed only if the port does not
 *              support any other method of detecting when the last ISR is
 *              executing.
 */
#define PORT_ISR_EXIT()                                                         \
    do {                                                                        \
        PortIsrNesting--;                                                       \
        esKernIsrExitI();                                                       \
    } while (0u)

/**@brief       If isrNesting variable is zero then the last ISR is executing
 *              and scheduler should be invoked
 * @return      Is the currently executed ISR the last one?
 *  @retval     TRUE - this is last ISR
 *  @retval     FALSE - this is not the last ISR
 */
#define PORT_ISR_IS_LAST()              (0u == PortIsrNesting ? TRUE : FALSE)

/**@} *//*----------------------------------------------------------------*//**
 * @name        System timer management
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Initialize system timer and associated interrupt
 * @param       val
 *              Value of system timer which will be loaded into the register
 * @details     This macro will only initialize system timer and associated
 *              interrupt. The macro is called from esKernStart() function.
 *              Responsibility:
 *              - initialize system timer
 *              - initialize system timer interrupt
 * @note        This macro MUST NOT enable system timer events. System timer
 *              events are enabled/disabled by PORT_SYSTMR_ISR_ENABLE() and
 *              PORT_SYSTMR_ISR_DISABLE() macros.
 */
#define PORT_SYSTMR_INIT(val)

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
 * @param       val
 *              Value of system timer which will be reloaded into the register
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

/**@} *//*----------------------------------------------------------------*//**
 * @name        Scheduler bit operations support
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Find last set bit in a word
 * @param       val
 *              Value : portReg_T, value which needs to be evaluated
 * @return      The position of the last set bit in a value
 * @details     This function is used by the scheduler to efficiently determine
 *              the highest priority of thread ready for execution. For similar
 *              algorithm details see:
 *              http://en.wikipedia.org/wiki/Find_first_set.
 */
#define PORT_BIT_FIND_LAST_SET(val)         portFindLastSet(val)

/**@brief       Helper macro: calculate 2^pwr expression
 * @param       pwr
 *              Power : portReg_T, value which will be used in calculation
 * @details     Some ports may want to use look up tables instead of shifting
 *              operation
 */
#define PORT_BIT_PWR2(pwr)                  (1U << (pwr))

/**@} *//*----------------------------------------------------------------*//**
 * @name        Dispatcher context switching
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Initialize the thread context
 * @param       [inout] stck
 *              Pointer to the allocated thread stack. The pointer points to the
 *              beginning of the memory as defined per C language. It's up to
 *              port function to adjust the pointer according to the stack type:
 *              full descending or full ascending one.
 * @param       stackSize
 *              The size of allocated stack in bytes.
 * @param       [in] thread
 *              Pointer to the thread function.
 * @param       [in] arg
 *              Argument that will be passed to thread function at the starting
 *              of execution.
 * @return      The new top of stack after thread context initialization.
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
#define PORT_CTX_SW_START()

/**@} *//*----------------------------------------------------------------*//**
 * @name        General port macros
 * @{ *//*--------------------------------------------------------------------*/

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

/**@brief       Terminate port
 * @details     This macro will be called when there is a need to stop any
 *              further execution (example: an error occured and CPU needs to
 *              stop).
 */
#define PORT_TERM()

/**@brief       Calculate the stack size
 * @details     This macro is used when specifying the size of thread stack.
 *              Responsibility:
 *              - add to @p size the minimal stack size specified by
 *              @ref PORT_DEF_DATA_WIDTH.
 *              - if it is needed by the port make sure the alignment is correct.
 */
#define PORT_STCK_SIZE(size)                                                    \
    ((((size + PORT_STCK_MINSIZE_VAL) + (sizeof(struct portStck) /                  \
    sizeof(portReg_T))) - 1U) / (sizeof(struct portStck)/sizeof(portReg_T)))

/**@brief       Exit critical section and enter sleep state
 */
#define PORT_CRITICAL_EXIT_SLEEP()                                              \
    portIntSetSleepEnter_(intStatus_)


/** @} *//*---------------------------------------------  C++ extern begin  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/

/**@brief       Data type which corresponds to the general purpose register
 * @details     Reading and writing this data type is guaranteed to happen in a
 *              single instruction, so there's no way for a handler to run “in
 *              the middle” of an access.
 *
 *              The type portReg_T is always an integer data type, but which one
 *              it is, and how many bits it contains, may vary from machine to
 *              machine.
 * @note        This data type will always have maximum number of bits which can
 *              be accessed atomically.
 */
typedef uint8_t portReg_T;

/**@brief       Stack structure used for stack declaration in order to force the
 *              alignment
 */
struct portStck {
    portReg_T       reg;                                                        /**< @brief A structure field representing stack data       */
};                                                                              /**< @brief Alignment of stack structure                    */

/**@brief       Stack type
 */
typedef struct portStck portStck_T;                                             /**< @brief Stack type                                      */

/**@brief       Port context structure
 */
struct portCtx {
    portReg_T       r0;                                                         /**< @brief Data pushed on stack during context switching   */
};

/*======================================================  GLOBAL VARIABLES  ==*/

/**@brief       Variable to keep track of ISR nesting.
 */
extern portReg_T PortIsrNesting;

/**@brief       Look up table for: 2^n expression
 * @details     This look up table can be used to accelerate the Logical Shift
 *              Left operations which are needed to set bits inside the priority
 *              bit map. In plain C this operation would be written as:
 *              <code>(1u << n)</code>, but in many 8-bit CPUs this operation
 *              can be lengthy. If there is a need for faster operation then
 *              this table can be used instead of the mentioned C code.
 *
 *              To use the look up table change @ref PORT_BIT_PWR2 macro
 *              implementation from: <code>(1u << (pwr))</code> to
 *              <code>pwr2LKP[pwr]</code>
 */
extern const PORT_C_ROM portReg_T Pwr2LKP [PORT_DEF_DATA_WIDTH];

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
 * @return      The position of the last set bit in a word
 * @details     This function is used by the scheduler to efficiently determine
 *              the highest priority of thread ready for execution. For
 *              algorithm details see:
 *              http://en.wikipedia.org/wiki/Find_first_set.
 */
uint_fast8_t portFindLastSet(
    portReg_T       val);

/**@} *//*----------------------------------------------------------------*//**
 * @name        Dispatcher context switching
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Initialize the thread context
 * @param       stck
 *              Pointer to the allocated thread stack. The pointer points to the
 *              beginning of the memory as defined per C language. It's up to
 *              port function to adjust the pointer according to the stack type:
 *              full descending or full ascending one.
 * @param       stckSize
 *              The size of allocated stack in bytes.
 * @param       fn
 *              Pointer to the thread function.
 * @param       arg
 *              Argument that will be passed to thread function at the starting
 *              of execution.
 * @return      The new top of stack after thread context initialization.
 */
void * portCtxInit(
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
