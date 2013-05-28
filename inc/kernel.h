/******************************************************************************
 * This file is part of eSolid-Kernel
 *
 * Copyright (C) 2011, 2012 - Nenad Radulovic
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
 * @brief       Interface of kernel.
 * @addtogroup  kern_intf
 *********************************************************************//** @{ */

#ifndef KERNEL_H_
#define KERNEL_H_

/*=========================================================  INCLUDE FILES  ==*/
#include "compiler.h"
#include "kernel_cfg.h"
#include "cpu.h"

/*===============================================================  MACRO's  ==*/

/*------------------------------------------------------------------------*//**
 * @name        Kernel identification and version number
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Identifies the underlying kernel version number
 * @details     RTOS identification and version (main [31:16] .sub [15:0])
 */
#define ES_KERNEL_VER                   0x10000UL

/**@brief       Kernel identification string
 */
#define ES_KERNEL_ID                    "eSolid Kernel v1.0"

/**@} *//*----------------------------------------------------------------*//**
 * @name        Critical section management
 * @{ *//*--------------------------------------------------------------------*/

/**@brief		Critical section status variable declaration
 */
#define ES_CRITICAL_DECL()				PORT_CRITICAL_DECL()

/**@brief		Enter a critical section
 */
#define ES_CRITICAL_ENTER()				PORT_CRITICAL_ENTER()

/**@brief		Exit from critical section
 */
#define ES_CRITICAL_EXIT()				PORT_CRITICAL_EXIT()

/*------------------------------------------------------------------------*//**
 * @name        Error checking
 * @brief       Some basic infrastructure for error checking
 * @details     These macros provide basic detection of errors. For more
 *              datails see @ref errors.
 * @{ *//*--------------------------------------------------------------------*/

#if (1U == CFG_API_VALIDATION) || defined(__DOXYGEN__)

/**@brief       Generic assert macro
 * @param       expr
 *              Expression which MUST be TRUE
 */
# define ES_ASSERT(expr)                                                        \
    do {                                                                        \
        if (!(expr)) {                                                          \
            userAssert(PORT_C_FUNC, #expr);                                     \
        }                                                                       \
    } while (0U)

/**@brief       Execute code to fulfill the contract
 * @param       expr
 *              Expression to be executed only if contracts need to be validated.
 */
# define ES_API_OBLIGATION(expr)        expr

/**@brief       Make sure the caller has fulfilled all contract preconditions
 * @param       expr
 *              Expression which MUST be satisfied
 */
# define ES_API_REQUIRE(expr)           ES_ASSERT(expr)

/**@brief       Make sure the callee has fulfilled all contract postconditions
 * @param       expr
 *              Expression which MUST be satisfied
 */
# define ES_API_ENSURE(expr)            ES_ASSERT(expr)

#else
# define ES_ASSERT(expr)                (void)0
# define ES_API_OBLIGATION(expr)        (void)0
# define ES_API_REQUIRE(expr)           (void)0
# define ES_API_ENSURE(expr)            (void)0
#endif

/**@} *//*----------------------------------------------  C++ extern begin  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/

/*------------------------------------------------------------------------*//**
 * @name        Thread management
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Thread structure
 * @details     A thread structure is a data structure used by kernel to
 *              maintain information about a thread. Each thread requires its
 *              own ID structure and the structure is allocated in user memory
 *              space (RAM). The address of the thread’s ID structure is
 *              provided to OS thread-related services.
 *
 *              Thread structure is used as thread ID and a thread is always
 *              referenced using this structure.
 * @api
 */
struct esThd {
/**@warning     This member must be on the first position within this structure
 *              because it is used by context switching code.
 */
    void *          stck;                                                       /**< @brief Pointer to thread's Top Of Stack                */

/**@brief       Thread linked List structure
 */
    struct thdL {
        struct esThdQ * q;                                                      /**< @brief Indicates which queue is used                   */
        struct esThd *  next;                                                   /**< @brief Next thread in linked list                      */
        struct esThd *  prev;                                                   /**< @brief Previous thread in linked list                  */
    } thdL;                                                                     /**< @brief Thread linked list                              */
    uint_fast8_t    prio;                                                       /**< @brief Thread current priority level                   */
    uint_fast8_t    cprio;                                                      /**< @brief Constant Thread Priority level                  */
    uint_fast8_t    qCnt;                                                       /**< @brief Quantum counter                                 */
    uint_fast8_t    qRld;                                                       /**< @brief Quantum counter reload value                    */
#if (1U == CFG_API_VALIDATION) || defined(__DOXYGEN__)
    portReg_T		signature;                                                  /**< @brief Thread structure signature, see @ref errors     */
#endif
};

/**@brief       Thread type
 */
typedef struct esThd esThd_T;

/**@} *//*----------------------------------------------------------------*//**
 * @name        Thread Queue management
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Priority Bit Map Group Index
 * @notapi
 */
# define PRIO_BM_GRP_INDX                                                    \
    ((CFG_SCHED_PRIO_LVL + PORT_DATA_WIDTH - 1U) / PORT_DATA_WIDTH)

/**@brief       Thread Queue structure
 * @api
 */
struct esThdQ {

/**@brief       Priority Bit Map structure
 */
    struct prioBM {
#if (1U != PRIO_BM_GRP_INDX) || defined(__DOXYGEN__)
        portReg_T       bitGrp;                                                 /**< @brief Bit group indicator                             */
#endif
        portReg_T       bit[PRIO_BM_GRP_INDX];                                  /**< @brief Bit priority indicator                          */
    }               prioOcc;                                                    /**< @brief Priority Occupancy                              */
    struct esThd *  grp[CFG_SCHED_PRIO_LVL];                                    /**< @brief Array of Group Head pointers to priority groups */
#if (1U == CFG_API_VALIDATION) || defined(__DOXYGEN__)
    portReg_T       signature;                                                  /**< @brief Thread Queue struct signature, see @ref errors  */
#endif
};

/**@brief       Thread queue type
 */
typedef struct esThdQ esThdQ_T;

/**@} *//*----------------------------------------------------------------*//**
 * @name        Kernel control block
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Kernel state enumeration
 * @details     For more details see: @ref states
 * @api
 */
enum esKernState {
    ES_KERN_RUN         = 0x00U,                                                /**< Kernel is active, multi-threading available            */
    ES_KERN_INTSRV_RUN  = 0x01U,                                                /**< Servicing an interrupt  return to ES_KERN_RUN state    */
    ES_KERN_LOCK        = 0x02U,                                                /**< Kernel is locked, no multi-threading                   */
    ES_KERN_INTSRV_LOCK = 0x03U,                                                /**< Servicing an interrupt, return to ES_KERN_LOCK state   */
    ES_KERN_INIT        = 0x04U,                                                /**< Kernel is in initialization state, no multi-threading  */
    ES_KERN_INACTIVE    = 0x05U                                                 /**< Kernel data structures are not initialized             */
};

/**@brief       Kernel state type
 */
typedef enum esKernState esKernState_T;

/**@brief       Kernel control block structure
 * @details     This structure holds important status data about the kernel.
 *              Since all data within the structure is somewhat related and
 *              accessed within the same pieces of code it was decided it is
 *              better to group all kernel data into the structure. This way the
 *              compiler can generate code that gets the address of the
 *              structure and then use relative indirect addressing to access
 *              all members of the structure. This results in more efficient
 *              code on architectures that have relative indirect addressing
 *              capability.
 * @api
 */
struct esKernCntl {
    struct esThd *      cthd;                                                   /**< @brief Pointer to the Current Thread                   */
    struct esThd *      pthd;                                                   /**< @brief Pointer to the Pending Thread to be switched    */
    enum esKernState    state;                                                  /**< @brief State of kernel                                 */
};

/**@brief       Kernel control block type
 */
typedef struct esKernCntl esKernCntl_T;

/**@} *//*--------------------------------------------------------------------*/
/*======================================================  GLOBAL VARIABLES  ==*/

/*------------------------------------------------------------------------*//**
 * @name        Kernel control block
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Kernel control block
 * @note        This variable has Read-Only access rights for application.
 */
extern const volatile esKernCntl_T gKernCntl;


/**@} *//*--------------------------------------------------------------------*/
/*===================================================  FUNCTION PROTOTYPES  ==*/

/*------------------------------------------------------------------------*//**
 * @name        General kernel functions
 * @details     There are several groups of functions:
 *              - kernel initialization and start
 *              - ISR prologue and epilogue
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Initialize kernel internal data structures
 * @details     This is the function which MUST be called first before any other
 *              kernel API. It initializes internal data structures which all
 *              other kernel API use.
 * @pre         1) <code>The kernel state == ES_KERN_INACTIVE</code>, see
 *                  @ref states.
 * @post        1) <code>The kernel state == ES_KERN_INIT</code>.
 * @note        1) This function may be invoked only once.
 * @api
 */
void esKernInit(
    void);

/**@brief       Start the multi-threading
 * @details     This function will start multi-threading. Once the
 *              multi-threading has started the execution will never return to
 *              this function again (this function never returns).
 * @pre         1) <code>The kernel state == ES_KERN_INIT</code>, see
 *                  @ref states.
 * @pre         2) At least one thread must be initialized and be in Threads
 *                  Ready Queue before starting multi-threading, see esThdInit().
 * @post        1) <code>The kernel state == ES_KERN_RUN</code>
 * @post        2) The multi-threading execution will commence.
 * @note        1) Once this function is called the execution of threads will
 *                  start and this function will never return.
 * @api
 */
void esKernStart(
    void);

/**@brief       Process the system timer event
 * @details     This function will be called only by port system timer interrupt.
 * @pre         1) <code>The kernel state < ES_KERN_INIT</code>, see
 *                  @ref states.
 * @notapi
 */
void esKernSysTmrI(
    void);

/**@brief       Enter Interrupt Service Routine
 * @details     Function will notify kernel that you are about to enter
 *              interrupt service routine (ISR). This allows kernel to keep
 *              track of interrupt nesting and then only perform rescheduling at
 *              the last nested ISR.
 *
 * @pre         1) <code>The kernel state < ES_KERN_INIT</code>, see @ref states.
 * @note        1) You MUST call esKernIsrEpilogueI() at the exit of ISR.
 * @note        2) You MUST invoke esKernIsrPrologueI() and esKernIsrEpilogueI()
 *                  in pair. In other words, for every call to
 *                  esKernIsrPrologueI() at the beginning of the ISR you MUST
 *                  have a call to esKernIsrEpilogueI() at the end of the ISR.
 * @iclass
 */
void esKernIsrPrologueI(
    void);

/**@brief       Exit Interrupt Service Routine
 * @details     This function is used to notify kernel that you have completed
 *              servicing an interrupt. When the last nested ISR has completed,
 *              the function will call the scheduler to determine whether a new,
 *              high-priority task, is ready to run.
 *
 * @pre         1) <code>The kernel state < ES_KERN_INIT</code>, see @ref states.
 * @note        1) You MUST invoke esKernIsrPrologueI() and esKernIsrEpilogueI()
 *                  in pair. In other words, for every call to
 *                  esKernIsrPrologueI() at the beginning of the ISR you MUST
 *                  have a call to esKernIsrEpilogueI() at the end of the ISR.
 * @note        2) Rescheduling is prevented when the scheduler is locked
 *                  (see esKernLockEnterI())
 * @iclass
 */
void esKernIsrEpilogueI(
    void);

/**@} *//*----------------------------------------------------------------*//**
 * @name        Critical section management
 * @details     These macros are used to prevent interrupts on entry into the
 *              critical section, and restoring interrupts to their previous
 *              state on exit from critical section.
 *
 *              For more details see @ref critical_section.
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       To do
 */
void esKernLockEnterI(
    void);

/**@brief       To do
 */
void esKernLockExitI(
    void);

/**@brief       To do
 */
void esKernLockIsrEnterI(
    void);

/**@brief       To do
 */
void esKernLockIsrExitI(
    void);

/**@} *//*----------------------------------------------------------------*//**
 * @name        Thread management
 * @brief       Basic thread management services
 * @details     For more details see @ref threads.
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Initialize the specified thread
 * @param       thd
 *              Thread: is a pointer to the thread structure, @ref esThd_T.
 *              The structure will be used as information container for the
 *              thread. It is assumed that storage for the esThd_T structure is
 *              allocated by the user code.
 * @param       thdf
 *              Thread Function: is a pointer to thread function. Thread
 *              function must have the following signature: <code>void thread
 *              (void * arg)</code>.
 * @param       arg
 *              Argument: is a void pointer to an optional data area. It's usage
 *              is application defined and it is intended to pass arguments to
 *              thread when it is started for the first time.
 * @param       stck
 *              Stack: is a void pointer to a allocated memory for thread stack.
 *              The pointer always points to the first element in the array,
 *              regardless of what type of stack the CPU is using. The thread's
 *              stack is used to store local variables, function parameters,
 *              return addresses. Each thread has its own stack and different
 *              sized stack.
 * @param       stckSize
 *              Stack Size: specifies the size of allocated stack memory. Size
 *              is expressed in bytes. Please see port documentation about
 *              minimal stack size.
 * @param       prio
 *              Priority is the priority of the thread. The higher the number,
 *              the higher the priority (the importance) of the thread. Several
 *              threads can have the same priority.
 * @details     Threads must be created in order for OS to recognize them as
 *              threads. Initialize a thread by calling esThdInit() and
 *              provide arguments specifying to OS how the thread will be
 *              managed. Threads are always created in the @c ready-to-run state.
 *              Threads can be created either prior to the start of
 *              multi-threading (before calling esKernStart()), or by a running
 *              thread.
 * @pre         1) <code>The kernel state < ES_KERN_INACTIVE</code>, see
 *                  @ref states.
 * @pre         2) <code>thd != NULL</code>
 * @pre         3) <code>thdf != NULL</code>
 * @pre         4) <code>stckSize >= PORT_STCK_MINSIZE</code>, see
 *                  @ref PORT_STCK_MINSIZE.
 * @pre         5) <code>0 <= prio <= CFG_SCHED_PRIO_LVL</code>, see
 *                  @ref CFG_SCHED_PRIO_LVL.
 * @post        1) <code>thd->signature == THD_CONTRACT_SIGNATURE</code>, each
 *                  esThd_T structure will have valid signature after
 *                  initialization.
 * @api
 */
void esThdInit(
    esThd_T *       thd,
    void (* thdf)(void *),
    void *          arg,
    void *          stck,
    size_t          stckSize,
    uint8_t         prio);

/**@brief       Terminate the specified thread
 * @param       thd
 *              Thread: is a pointer to the thread structure, @ref esThd_T.
 * @pre         1) <code>The kernel state < ES_KERN_INACTIVE</code>, see
 *                  @ref states.
 * @pre         2) <code>thd != NULL</code>
 * @pre         3) <code>thd->signature == THD_CONTRACT_SIGNATURE</code>, the
 *                  pointer must point to a @ref esThd_T structure.
 */
void esThdTerm(
    esThd_T *       thd);

/**@brief       Get the current thread ID
 * @return      Pointer to current thread ID structure @ref esThd.
 * @inline
 */
static PORT_C_INLINE esThd_T * esThdGetId(
    void) {

    return (gKernCntl.cthd);
}

/**@brief       Get the priority of a thread
 * @param       thd
 *              Thread: is a pointer to the thread structure, @ref esThd_T.
 * @return      The priority of the thread pointed by @c thd.
 * @inline
 */
static PORT_C_INLINE uint8_t esThdGetPrio(
    esThd_T *       thd) {

    return (thd->prio);
}

/**@brief       Set the priority of a thread
 * @param       thd
 *              Thread: is a pointer to the thread structure, @ref esThd_T.
 * @param       prio
 *              Priority: is new priority of the thread pointed by @c thd.
 * @pre         1) <code>The kernel state < ES_KERN_INACTIVE</code>, see
 *                  @ref states.
 * @pre         2) <code>thd != NULL</code>
 * @pre         3) <code>thd->signature == THD_CONTRACT_SIGNATURE</code>, the
 *                  pointer must point to a @ref esThd_T structure.
 * @pre         4) <code>0 <= prio <= CFG_SCHED_PRIO_LVL</code>, see
 *                  @ref CFG_SCHED_PRIO_LVL.
 * @iclass
 */
void esThdSetPrioI(
    esThd_T *       thd,
    uint8_t         prio);

/**@} *//*----------------------------------------------------------------*//**
 * @name        Thread Queue management
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Initialize Thread Queue
 * @param       thdQ
 *              Thread Queue: is a pointer to thread queue structure,
 *              @ref esThdQ.
 * @pre         1) <code>thdQ != NULL</code>
 * @post        1) <code>thdQ->signature == THDQ_CONTRACT_SIGNATURE</code>, each
 *                  esThdQ_T structure will have valid signature after
 *                  initialization.
 * @api
 */
void esThdQInit(
    esThdQ_T *      thdQ);

/**@brief       Add a thread to the tail of the Thread Queue
 * @param       thdQ
 *              Thread Queue: is a pointer to thread queue structure,
 *              @ref esThdQ.
 * @param       thd
 *              Thread: is a pointer to the thread ID structure, @ref esThd_T.
 * @details     This function adds a thread at the tail of the specified Thread
 *              Queue.
 * @pre         1) <code>thdQ != NULL</code>
 * @pre         2) <code>thdQ->signature == THDQ_CONTRACT_SIGNATURE</code>, the
 *                  pointer must point to a @ref esThdQ_T structure.
 * @pre         3) <code>thd != NULL</code>
 * @pre         4) <code>thd->signature == THD_CONTRACT_SIGNATURE</code>, the
 *                  pointer must point to a @ref esThd_T structure.
 * @pre         5) <code>thd->thdL.q == NULL</code>, thread MUST NOT be in any
 *                  queue.
 * @pre         6) <code>0 <= thd->prio <= CFG_SCHED_PRIO_LVL</code>, see
 *                  @ref CFG_SCHED_PRIO_LVL.
 * @iclass
 */
void esThdQAddI(
    esThdQ_T *      thdQ,
    esThd_T *       thd);

/**@brief       Removes the thread from the Thread Queue
 * @param       thdQ
 *              Thread Queue: is a pointer to thread queue structure,
 *              @ref esThdQ.
 * @param       thd
 *              Thread: is a pointer to the thread ID structure, @ref esThd_T.
 * @pre         1) <code>thd != NULL</code>
 * @pre         2) <code>thd->signature == THD_CONTRACT_SIGNATURE</code>, the
 *                  pointer must point to a @ref esThd_T structure.
 * @pre         3) <code>thdQ != NULL</code>
 * @pre         4) <code>thdQ->signature == THDQ_CONTRACT_SIGNATURE</code>, the
 *                  pointer must point to a @ref esThdQ_T structure.
 * @pre         5) <code>thd->thdL.q == thdQ</code>, thread MUST BE in the queue.
 * @pre         6) <code>0 <= thd->prio <= CFG_SCHED_PRIO_LVL</code>, see
 *                  @ref CFG_SCHED_PRIO_LVL.
 * @iclass
 */
void esThdQRmI(
    esThdQ_T *      thdQ,
    esThd_T *       thd);

/**@brief       Fetch the first high priority thread from the Thread Queue
 * @param       thdQ
 *              Thread Queue: is a pointer to thread queue structure,
 *              @ref esThdQ.
 * @return      A pointer to the thread ID structure with the highest priority.
 * @pre         1) <code>thdQ != NULL</code>
 * @pre         2) <code>thdQ->signature == THDQ_CONTRACT_SIGNATURE</code>, the
 *                  pointer must point to a @ref esThdQ_T structure.
 * @iclass
 */
esThd_T * esThdQFetchFirstI(
    const esThdQ_T *    thdQ);

/**@brief       Fetch the next thread and rotate linked list
 * @param       thdQ
 *              Thread Queue: is a pointer to thread queue structure,
 *              @ref esThdQ. This is the thread queue to fetch from.
 * @param       prio
 *              Priority: is the priority level to fetch and rotate.
 * @return      Pointer to the next thread in queue.
 * @pre         1) <code>thdQ != NULL</code>
 * @pre         2) <code>thdQ->signature == THDQ_CONTRACT_SIGNATURE</code>, the
 *                  pointer must point to a @ref esThdQ_T structure.
 * @pre         3) <code>0 <= prio <= CFG_SCHED_PRIO_LVL</code>, see
 *                  @ref CFG_SCHED_PRIO_LVL.
 */
esThd_T * esThdQRotateI(
    esThdQ_T *      thdQ,
    uint_fast8_t    prio);

/**@brief       Is thread queue empty
 * @param       thdQ
 *              Thread Queue: is a pointer to thread queue structure,
 *              @ref esThdQ.
 * @return      The state of thread queue
 *  @retval     TRUE - thread queue is empty
 *  @retval     FALSE - thread queue is not empty
 * @pre         1) <code>thdQ != NULL</code>
 * @pre         2) <code>thdQ->signature == THDQ_CONTRACT_SIGNATURE</code>, the
 *                  pointer must point to a @ref esThdQ_T structure.
 */
bool_T esThdQIsEmpty(
    const esThdQ_T *    thdQ);

/**@} *//*----------------------------------------------------------------*//**
 * @name        Scheduler notification and invocation
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Add thread @c thd to the ready thread list and notify the
 *              scheduler.
 * @param       thd
 *              Pointer to the initialized thread ID structure, @ref esThd_T.
 * @pre         1) <code>The kernel state < ES_KERN_INACTIVE</code>, see
 *                  @ref states.
 * @pre         2) <code>thd != NULL</code>
 * @pre         3) <code>thd->signature == THD_CONTRACT_SIGNATURE</code>, the
 *                  pointer must point to a @ref esThd_T structure.
 * @pre         3) <code>thd->thdL.q == NULL</code>, thread MUST NOT be in a
 *              queue.
 * @iclass
 */
void esSchedRdyAddI(
    esThd_T *       thd);

/**@brief       Remove thread @c thd from the ready thread list and notify
 *              the scheduler.
 * @param       thd
 *              Pointer to the initialized thread ID structure, @ref esThd_T.
 * @pre         1) <code>The kernel state < ES_KERN_INACTIVE</code>, see
 *                  @ref states.
 * @pre         2) <code>thd != NULL</code>
 * @pre         3) <code>thd->signature == THD_CONTRACT_SIGNATURE</code>, the
 *                  pointer must point to a @ref esThd_T structure.
 * @pre         4) <code>thd->thdL.q == &gRdyQueue.thdQ</code>, thread MUST BE
 *              in Ready Threads queue.
 * @iclass
 */
void esSchedRdyRmI(
    esThd_T *       thd);

/**@brief       Force the scheduler invocation which will evaluate all ready
 *              threads and switch to ready thread with the highest priority
 * @pre         1) <code>The kernel state < ES_KERN_INACTIVE</code>, see
 *                  @ref states.
 * @warning     Scheduler will have undefined behavior if there is no ready
 *              thread to run (e.g. empty @ref sched_rdyThdQ) at the time it is
 *              invoked.
 * @iclass
 */
void esSchedYieldI(
    void);

/**@brief       Force the scheduler invocation which will evaluate all ready
 *              threads and switch to ready thread with the highest priority
 * @pre         1) <code>The kernel state < ES_KERN_INACTIVE</code>, see
 *                  @ref states.
 * @warning     Scheduler will have undefined behavior if there is no ready
 *              thread to run (e.g. empty @ref sched_rdyThdQ) at the time it is
 *              invoked.
 * @iclass
 */
void esSchedYieldIsrI(
    void);

/**@} *//*----------------------------------------------------------------*//**
 * @name        Kernel hook functions
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       An assertion has failed. This function should inform the user
 *              about failed assertion.
 * @param       funcName
 *              Function name: pointer to the function name string where the
 *              assertion failed. Macro will automatically fill in the function
 *              name for the function.
 * @param       expr
 *              Expression: pointer to the string containing the expression that
 *              failed.
 * @details     Function should just print the information which was given by
 *              the macros. After the function informs the user it MUST go into
 *              infinite loop or HALT the processor.
 * @pre         1) <code>NULL != funcName</code>
 * @pre         2) <code>NULL != expr</code>
 * @note        1) The definition of this function must be written by the user.
 * @note        2) This function is called only if @ref CFG_API_VALIDATION is
 *              active.
 */
extern void userAssert(
    const char *    funcName,
    const char *    expr);

/**@brief       System timer hook function, called from system system timer ISR
 *              function.
 * @details     This function is called whenever a system event is generated.
 * @note        1) The definition of this function must be written by the user.
 * @note        2) This function is called only if @ref CFG_HOOK_SYSTMR_EVENT is
 *              active.
 */
extern void userSysTmr(
    void);

/**@brief       Kernel initialization hook function, called from esKernInit()
 *              function.
 * @details     This function is called before kernel initialization.
 * @note        1) The definition of this function must be written by the user.
 * @note        2) This function is called only if @ref CFG_HOOK_KERN_INIT is
 *              active.
 */
extern void userKernInit(
    void);

/**@brief       Kernel start hook function, called from esKernStart() function.
 * @details     This function is called before kernel start.
 * @note        1) The definition of this function must be written by the user.
 * @note        2) This function is called only if @ref CFG_HOOK_KERN_START is
 *              active.
 */
extern void userKernStart(
    void);

/**@brief       Kernel thread init end hook function, called from esThdInit()
 *              function.
 * @details     This function is called after the thread initialization.
 * @note        1) The definition of this function must be written by the user.
 * @note        2) This function is called only if @ref CFG_HOOK_THD_INIT_END is
 *              active.
 */
extern void userThdInitEnd(
    void);

/**@brief       Kernel context switch hook function, called from esSchedYieldI()
 *              and esSchedYieldIsrI() functions.
 * @details     This function is called at each context switch.
 * @param       oldThd
 *              Pointer to the thread being switched out.
 * @param       newThd
 *              Pointer to the thread being switched in.
 * @note        1) The definition of this function must be written by the user.
 * @note        2) This function is called only if @ref CFG_HOOK_CTX_SW is
 *              active.
 */
extern void userCtxSw(
    esThd_T *       oldThd,
    esThd_T *       newThd);

/**@} *//*------------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//**@} *//*******************************************************
 * END of kernel.h
 ******************************************************************************/
#endif /* KERNEL_H_ */
