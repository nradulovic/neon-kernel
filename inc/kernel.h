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
 * @brief       Interface of kernel.
 * @addtogroup  kern_intf
 *********************************************************************//** @{ */

#ifndef KERNEL_H_
#define KERNEL_H_

/*=========================================================  INCLUDE FILES  ==*/
#include "compiler.h"
#include "kernel_cfg.h"
#include "cpu.h"
#include "list.h"

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
 *              Expression which must be TRUE
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
 *              Expression which must be satisfied
 */
# define ES_API_REQUIRE(expr)           ES_ASSERT(expr)

/**@brief       Make sure the callee has fulfilled all contract postconditions
 * @param       expr
 *              Expression which must be satisfied
 */
# define ES_API_ENSURE(expr)            ES_ASSERT(expr)

#else
# define ES_ASSERT(expr)                (void)0
# define ES_API_OBLIGATION(expr)        (void)0
# define ES_API_REQUIRE(expr)           (void)0
# define ES_API_ENSURE(expr)            (void)0
#endif

/**@} *//*----------------------------------------------------------------*//**
 * @name        Thread management
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Converts the required stack elements into the stack array index
 */
#define ES_STCK_SIZE(elem)              PORT_STCK_SIZE(elem)

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
    portStck_T *    stck;                                                       /**< @brief Pointer to thread's Top Of Stack                */

/**@brief       Thread linked List structure
 */
    struct thdL {
        struct esThdQ * q;                                                      /**< @brief Indicates which queue is used                   */
        struct esThd *  next;                                                   /**< @brief Next thread in linked list                      */
        struct esThd *  prev;                                                   /**< @brief Previous thread in linked list                  */
    }               thdL;                                                       /**< @brief Thread linked list                              */
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

/**@brief       Stack type
 */
typedef portStck_T esStck_T;

/**@} *//*----------------------------------------------------------------*//**
 * @name        Timer services
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Timer structure
 */
struct esTmr {

/**@brief       Timer linked list structure
 */
    struct tmrL {
        struct esTmr *  next;                                                   /**< @brief Next thread in timer linked list                */
        struct esTmr *  prev;                                                   /**< @brief Previous thread in timer linked list            */
    }               tmrL;                                                       /**< @brief Timer linked List                               */
    esTick_T        rtick;                                                      /**< @brief Relative tick value                             */
    void (* fn)(void *);                                                        /**< @brief Callback function pointer                       */
    void *          arg;                                                        /**< @brief Callback function argument                      */
#if (1U == CFG_API_VALIDATION) || defined(__DOXYGEN__)
    portReg_T       signature;                                                  /**< @brief Timer structure signature, see @ref errors      */
#endif
};

/**@brief       Timer type
 */
typedef struct esTmr esTmr_T;

/**@} *//*----------------------------------------------------------------*//**
 * @name        Thread Queue management
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Priority Bit Map Group Index
 * @notapi
 */
# define PRIO_BM_GRP_INDX                                                       \
    ((CFG_SCHED_PRIO_LVL + PORT_DATA_WIDTH_VAL - 1U) / PORT_DATA_WIDTH_VAL)

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
    ES_KERN_RUN         = 0x00U,                                                /**< Kernel is active                                       */
    ES_KERN_INTSRV_RUN  = 0x01U,                                                /**< Servicing an interrupt  return to ES_KERN_RUN state    */
    ES_KERN_LOCK        = 0x02U,                                                /**< Kernel is locked                                       */
    ES_KERN_INTSRV_LOCK = 0x03U,                                                /**< Servicing an interrupt, return to ES_KERN_LOCK state   */
    ES_KERN_INIT        = 0x04U,                                                /**< Kernel is in initialization state                      */
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
struct esKernCtrl {
    struct esThd *      cthd;                                                   /**< @brief Pointer to the Current Thread                   */
    struct esThd *      pthd;                                                   /**< @brief Pointer to the Pending Thread to be switched    */
    enum esKernState    state;                                                  /**< @brief State of kernel                                 */
};

/**@brief       Kernel control block type
 */
typedef struct esKernCtrl esKernCtrl_T;

/**@} *//*--------------------------------------------------------------------*/
/*======================================================  GLOBAL VARIABLES  ==*/

/*------------------------------------------------------------------------*//**
 * @name        Kernel control block
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Kernel control block
 * @note        This variable has Read-Only access rights for application.
 */
extern const volatile esKernCtrl_T gKernCtrl;

/**@} *//*--------------------------------------------------------------------*/
/*===================================================  FUNCTION PROTOTYPES  ==*/

/*------------------------------------------------------------------------*//**
 * @name        General kernel functions
 * @details     There are several groups of functions:
 *              - kernel initialization and start
 *              - ISR prologue and epilogue
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Initialize kernel internal data structures
 * @details     This is the function which must be called first before any other
 *              kernel API. It initializes internal data structures which all
 *              other kernel API use.
 * @pre         1) `The kernel state == ES_KERN_INACTIVE`, see @ref states.
 * @post        1) `The kernel state == ES_KERN_INIT`.
 * @note        1) This function may be invoked only once.
 * @api
 */
void esKernInit(
    void);

/**@brief       Start the multi-threading
 * @details     This function will start multi-threading. Once the
 *              multi-threading has started the execution will never return to
 *              this function again (this function never returns).
 * @pre         1) `The kernel state == ES_KERN_INIT`, see @ref states.
 * @pre         2) At least one thread must be initialized and be in Threads
 *                  Ready Queue before starting multi-threading, see esThdInit().
 * @post        1) `The kernel state == ES_KERN_RUN`
 * @post        2) The multi-threading execution will commence.
 * @note        1) Once this function is called the execution of threads will
 *                  start and this function will never return.
 * @api
 */
PORT_C_NORETURN void esKernStart(
    void);

/**@brief       Process the system timer event
 * @details     This function will be called only by port system timer interrupt.
 * @pre         1) `The kernel state < ES_KERN_INIT`, see @ref states.
 * @notapi
 */
void esKernSysTmrI(
    void);

/**@brief       Enter Interrupt Service Routine
 * @details     Function will notify kernel that you are about to enter
 *              interrupt service routine (ISR). This allows kernel to keep
 *              track of interrupt nesting and then only perform rescheduling at
 *              the last nested ISR.
 * @pre         1) `The kernel state < ES_KERN_INIT`, see @ref states.
 * @note        1) You must call esKernIsrEpilogueI() at the exit of ISR.
 * @note        2) You must invoke esKernIsrPrologueI() and esKernIsrEpilogueI()
 *                  in pair. In other words, for every call to
 *                  esKernIsrPrologueI() at the beginning of the ISR you must
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
 * @pre         1) `The kernel state < ES_KERN_INIT`, see @ref states.
 * @note        1) You must invoke esKernIsrPrologueI() and esKernIsrEpilogueI()
 *                  in pair. In other words, for every call to
 *                  esKernIsrPrologueI() at the beginning of the ISR you must
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

/**@brief       Lock the scheduler
 * @iclass
 */
void esKernLockEnterI(
    void);

/**@brief       Unlock the scheduler
 * @iclass
 */
void esKernLockExitI(
    void);

/**@brief       Lock the scheduler
 * @api
 */
void esKernLockEnter(
    void);

/**@brief       Unlock the scheduler
 * @api
 */
void esKernLockExit(
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
 *              function must have the following signature: `void thread
 *              (void * arg)`.
 * @param       arg
 *              Argument: is a void pointer to an optional data area. It's usage
 *              is application defined and it is intended to pass arguments to
 *              thread when it is started for the first time.
 * @param       stck
 *              Stack: is a pointer to a allocated memory for thread stack.
 *              The pointer always points to the first element in the array,
 *              regardless of what type of stack the CPU is using. The thread's
 *              stack is used to store local variables, function parameters,
 *              return addresses. Each thread has its own stack and different
 *              sized stack. The stack type must be an array of @ref portStck_T.
 * @param       stckSize
 *              Stack Size: specifies the size of allocated stack memory. Size
 *              is expressed in bytes. Please see port documentation about
 *              minimal stack size.
 * @param       prio
 *              Priority: is the priority of the thread. The higher the number,
 *              the higher the priority (the importance) of the thread. Several
 *              threads can have the same priority.
 * @details     Threads must be created in order for kernel to recognize them as
 *              threads. Initialize a thread by calling esThdInit() and
 *              provide arguments specifying to kernel how the thread will be
 *              managed. Threads are always created in the @c ready-to-run state.
 *              Threads can be created either prior to the start of
 *              multi-threading (before calling esKernStart()), or by a running
 *              thread.
 * @pre         1) `The kernel state ES_KERN_INACTIVE`, see @ref states.
 * @pre         2) `thd != NULL`
 * @pre         3) `thdf != NULL`
 * @pre         4) `stckSize >= PORT_STCK_MINSIZE_VAL`, see
 *                  @ref PORT_STCK_MINSIZE_VAL.
 * @pre         5) `0 <= prio <= CFG_SCHED_PRIO_LVL`, see
 *                  @ref CFG_SCHED_PRIO_LVL.
 * @post        1) `thd->signature == THD_CONTRACT_SIGNATURE`, each esThd_T
 *                  structure will have valid signature after initialization.
 * @api
 */
void esThdInit(
    esThd_T *       thd,
    void (* thdf)(void *),
    void *          arg,
    portStck_T *    stck,
    size_t          stckSize,
    uint8_t         prio);

/**@brief       Terminate the specified thread
 * @param       thd
 *              Thread: is a pointer to the thread structure, @ref esThd_T.
 * @pre         1) `The kernel state ES_KERN_INACTIVE`, see @ref states.
 * @pre         2) `thd != NULL`
 * @pre         3) `thd->signature == THD_CONTRACT_SIGNATURE`, the pointer must
 *                  point to a @ref esThd_T structure.
 * @pre         4) `(thd->thdL.q == NULL) OR (thd->thdL.q == gRdyQueue)`, thread
 *                  must be either in Ready Threads Queue or not be in any queue
 *                  (e.g. not waiting for a synchronization mechanism).
 */
void esThdTerm(
    esThd_T *       thd);

/**@brief       Get the current thread ID
 * @return      Pointer to current thread ID structure @ref esThd.
 * @inline
 * @api
 */
static PORT_C_INLINE esThd_T * esThdGetId(
    void) {

    return (gKernCtrl.cthd);
}

/**@brief       Get the priority of a thread
 * @param       thd
 *              Thread: is pointer to the thread structure, @ref esThd_T.
 * @return      The priority of the thread pointed by @c thd.
 * @inline
 * @api
 */
static PORT_C_INLINE uint8_t esThdGetPrio(
    esThd_T *       thd) {

    return (thd->prio);
}

/**@brief       Set the priority of a thread
 * @param       thd
 *              Thread: is pointer to the thread structure, @ref esThd_T.
 * @param       prio
 *              Priority: is new priority of the thread pointed by @c thd.
 * @pre         1) `The kernel state < ES_KERN_INACTIVE`, see @ref states.
 * @pre         2) `thd != NULL`
 * @pre         3) `thd->signature == THD_CONTRACT_SIGNATURE`, the pointer must
 *                  point to a @ref esThd_T structure.
 * @pre         4) `0 <= prio <= CFG_SCHED_PRIO_LVL`, see
 *                  @ref CFG_SCHED_PRIO_LVL.
 * @iclass
 */
void esThdSetPrioI(
    esThd_T *       thd,
    uint8_t         prio);

/**@brief       Post to thread semaphore
 * @param       thd
 *              Pointer to the thread ID structure
 * @iclass
 */
void esThdPostI(
    esThd_T *       thd);

/**@brief       Post to thread semaphore
 * @param       thd
 *              Pointer to the thread ID structure
 * @api
 */
void esThdPost(
    esThd_T *       thd);

/**@brief       Wait for thread semaphore
 * @iclass
 */
void esThdWaitI(
    void);

/**@brief       Wait for thread semaphore
 * @api
 */
void esThdWait(
    void);

/**@} *//*----------------------------------------------------------------*//**
 * @name        Thread Queue management
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Initialize Thread Queue
 * @param       thdQ
 *              Thread Queue: is a pointer to thread queue structure,
 *              @ref esThdQ.
 * @pre         1) `thdQ != NULL`
 * @post        1) `thdQ->signature == THDQ_CONTRACT_SIGNATURE`, each
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
 * @pre         1) `thdQ != NULL`
 * @pre         2) `thdQ->signature == THDQ_CONTRACT_SIGNATURE`, the pointer
 *                  must point to a @ref esThdQ_T structure.
 * @pre         3) `thd != NULL`
 * @pre         4) `thd->signature == THD_CONTRACT_SIGNATURE`, the pointer must
 *                  point to a @ref esThd_T structure.
 * @pre         5) `thd->thdL.q == NULL`, thread must not be in any queue.
 * @pre         6) `0 <= thd->prio <= CFG_SCHED_PRIO_LVL`, see
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
 * @pre         1) `thd != NULL`
 * @pre         2) `thd->signature == THD_CONTRACT_SIGNATURE`, the pointer must
 *                  point to a @ref esThd_T structure.
 * @pre         3) `thdQ != NULL`
 * @pre         4) `thdQ->signature == THDQ_CONTRACT_SIGNATURE`, the pointer
 *                  must point to a @ref esThdQ_T structure.
 * @pre         5) `thd->thdL.q == thdQ`, thread must be in the `thdQ` queue.
 * @pre         6) `0 <= thd->prio <= CFG_SCHED_PRIO_LVL`, see
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
 * @pre         1) `thdQ != NULL`
 * @pre         2) `thdQ->signature == THDQ_CONTRACT_SIGNATURE`, the pointer
 *                  must point to a @ref esThdQ_T structure.
 * @iclass
 */
esThd_T * esThdQFetchI(
    const esThdQ_T *    thdQ);

/**@brief       Fetch the next thread and rotate linked list
 * @param       thdQ
 *              Thread Queue: is a pointer to thread queue structure,
 *              @ref esThdQ. This is the thread queue to fetch from.
 * @param       prio
 *              Priority: is the priority level to fetch and rotate.
 * @return      Pointer to the next thread in queue.
 * @pre         1) `thdQ != NULL`
 * @pre         2) `thdQ->signature == THDQ_CONTRACT_SIGNATURE`, the pointer
 *                  must point to a @ref esThdQ_T structure.
 * @pre         3) `0 <= prio <= CFG_SCHED_PRIO_LVL`, see
 *                  @ref CFG_SCHED_PRIO_LVL.
 */
esThd_T * esThdQFetchRotateI(
    esThdQ_T *      thdQ,
    uint_fast8_t    prio);

/**@brief       Is thread queue empty
 * @param       thdQ
 *              Thread Queue: is a pointer to thread queue structure,
 *              @ref esThdQ.
 * @return      The state of thread queue
 *  @retval     TRUE - thread queue is empty
 *  @retval     FALSE - thread queue is not empty
 * @pre         1) `thdQ != NULL`
 * @pre         2) `thdQ->signature == THDQ_CONTRACT_SIGNATURE`, the pointer
 *                  must point to a @ref esThdQ_T structure.
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
 * @pre         1) `The kernel state < ES_KERN_INACTIVE`, see @ref states.
 * @pre         2) `thd != NULL`
 * @pre         3) `thd->signature == THD_CONTRACT_SIGNATURE`, the pointer must
 *                  point to a @ref esThd_T structure.
 * @pre         3) `thd->thdL.q == NULL`, thread must not be in a queue.
 * @iclass
 */
void esSchedRdyAddI(
    esThd_T *       thd);

/**@brief       Remove thread @c thd from the ready thread list and notify
 *              the scheduler.
 * @param       thd
 *              Pointer to the initialized thread ID structure, @ref esThd_T.
 * @pre         1) `The kernel state < ES_KERN_INACTIVE`, see @ref states.
 * @pre         2) `thd != NULL`
 * @pre         3) `thd->signature == THD_CONTRACT_SIGNATURE`, the pointer must
 *                  point to a @ref esThd_T structure.
 * @pre         4) `thd->thdL.q == &gRdyQueue`, thread must be in Ready Threads
 *                  queue.
 * @iclass
 */
void esSchedRdyRmI(
    esThd_T *       thd);

/**@brief       Force the scheduler invocation which will evaluate all ready
 *              threads and switch to ready thread with the highest priority
 * @pre         1) `The kernel state < ES_KERN_INACTIVE`, see @ref states.
 * @warning     Scheduler will have undefined behavior if there is no ready
 *              thread to run (e.g. empty @ref gRdyQueue) at the time it is
 *              invoked.
 * @iclass
 */
void esSchedYieldI(
    void);

/**@brief       Force the scheduler invocation which will evaluate all ready
 *              threads and switch to ready thread with the highest priority
 * @pre         1) `The kernel state < ES_KERN_INACTIVE`, see @ref states.
 * @warning     Scheduler will have undefined behavior if there is no ready
 *              thread to run (e.g. empty @ref gRdyQueue) at the time it is
 *              invoked.
 * @iclass
 */
void esSchedYieldIsrI(
    void);

/**@} *//*----------------------------------------------------------------*//**
 * @name        System timer
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Enable system timer tick events
 * @details     This function will override scheduler power savings algorithm
 *              and force the system timer into running (active) state.
 */
void esSysTmrEnable(
    void);

/**@brief       Disable system timer tick events
 * @details     This function will try to switch off the system timer. If the
 *              system timer is used by scheduler than the scheduler will take
 *              control of the system timer.
 */
void esSysTmrDisable(
    void);

/**@} *//*----------------------------------------------------------------*//**
 * @name        Timer services
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Add a new timer
 * @param       tmr
 *              Timer: is pointer to the timer ID structure, @ref esTmr.
 * @param       tick
 *              Tick: the timer delay expressed in system ticks
 * @param       fn
 *              Function: is pointer to the callback function
 * @param       arg
 *              Argument: is pointer to the arguments of callback function
 */
void esTmrAddI(
    esTmr_T *       tmr,
    esTick_T        tick,
    void (* fn)(void *),
    void *          arg);

/**@} *//*----------------------------------------------------------------*//**
 * @name        Kernel hook functions
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       An assertion has failed. This function should inform the user
 *              about failed assertion.
 * @param       fnName
 *              Function name: is pointer to the function name string where the
 *              assertion has failed. Macro will automatically fill in the
 *              function name.
 * @param       expr
 *              Expression: is pointer to the string containing the expression
 *              that failed to evaluate to `TRUE`.
 * @details     Function will just print the information which was given by
 *              the macros. After the function informs the user it **must** go
 *              into infinite loop or HALT the processor.
 * @pre         1) `NULL != fnName`
 * @pre         2) `NULL != expr`
 * @note        1) The definition of this function must be written by the user.
 * @note        2) This function is called only if @ref CFG_API_VALIDATION is
 *              active.
 */
PORT_C_NORETURN extern void userAssert(
    const char *    fnName,
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

/**@brief       Thread initialization end hook function, called from esThdInit()
 *              function.
 * @details     This function is called after the thread initialization.
 * @note        1) The definition of this function must be written by the user.
 * @note        2) This function is called only if @ref CFG_HOOK_THD_INIT_END is
 *              active.
 */
extern void userThdInitEnd(
    void);

/**@brief       Thread terminate hook function, called from esThdTerm() or when
 *              a thread terminates itself.
 * @note        1) The definition of this function must be written by the user.
 * @note        2) This function is called only if @ref CFG_HOOK_THD_TERM is
 *              active.
 */
extern void userThdTerm(
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
