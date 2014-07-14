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

/**@brief       This macro specifies the minimal size of the thread stack
 * @details     Generally minimal stack size is equal to the size of context
 *              structure
 */
#define PORT_DEF_STCK_MINSIZE                                                   \
    (sizeof(struct portCtx) / sizeof(portReg_T))

/**@} *//*----------------------------------------------------------------*//**
 * @name        System timer constants
 * @{ *//*--------------------------------------------------------------------*/

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
#define NPORT_CTX_SW_START()

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
#define PORT_KCORE_INIT_EARLY()

/**@brief       Port initialization
 * @details     This macro will be called after kernel data structure
 *              initialization from esKernInit() function.
 */
#define PORT_KCORE_INIT()

/**@brief       Late port initialization
 * @details     This macro will be called just a moment before the multitasking
 *              is started. The macro is called from esKernStart() function.
 */
#define PORT_KCORE_INIT_LATE()

/**@brief       Terminate port
 * @details     This macro will be called when there is a need to stop any
 *              further execution (example: an error occurred and CPU needs to
 *              stop).
 */
#define PORT_KCORE_TERM()

/**@brief       Calculate the stack size
 * @details     This macro is used when specifying the size of thread stack.
 *              Responsibility:
 *              - add to @p size the minimal stack size specified by
 *              `PORT_DEF_DATA_WIDTH` macro.
 *              - if it is needed by the port make sure the alignment is correct.
 */
#define PORT_STCK_SIZE(size)                                                    \
    (size + PORT_DEF_STCK_MINSIZE)

/**@brief       Exit critical section and enter sleep state
 */
#define PORT_CRITICAL_EXIT_SLEEP()                                              \
    portIntSetSleepEnter_(intStatus_)


/** @} *//*---------------------------------------------  C++ extern begin  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/

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

/*===================================================  FUNCTION PROTOTYPES  ==*/

/*------------------------------------------------------------------------*//**
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
