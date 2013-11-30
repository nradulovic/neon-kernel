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
 * @brief       Main kernel interface
 * @defgroup    kern Kernel
 * @brief       Kernel overview
 * @details     Main kernel interface is divided into several sections.
 *********************************************************************//** @{ */
/**@defgroup    kern_intf Interface
 * @brief       Kernel main interface
 * @{ *//*--------------------------------------------------------------------*/

#if !defined(KERNEL_H__)
#define KERNEL_H__

/*=========================================================  INCLUDE FILES  ==*/

#include "arch/kcore.h"
#include "base/bitop.h"
#include "base/dbg.h"
#include "kernel/kernel_cfg.h"

/*===============================================================  MACRO's  ==*/

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
#define ES_KERN_ID                      "eSolid - RT Kernel"

/**@} *//*----------------------------------------------------------------*//**
 * @defgroup    kern_thd Thread
 * @brief       Thread management services
 * @details     For more details see @ref threads.
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Converts the required stack elements into the stack array index.
 * @param       elem
 *              Number of stack elements: the stack size is expressed in number
 *              of elements regardless of the size of port general purpose
 *              registers.
 * @return      Number of stack elements needed for stack usage.
 */
#define ES_STCK_SIZE(elem)              PORT_STCK_SIZE(elem)

/**@brief       Maximum level of priority possible for application thread
 */
#define ES_DEF_THD_PRIO_MAX             (CFG_SCHED_PRIO_LVL - 2u)

/**@brief       Minimum level of priority possible for application thread
 */
#define ES_DEF_THD_PRIO_MIN             (1u)

/**@} *//*----------------------------------------------  C++ extern begin  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/

/*------------------------------------------------------------------------*//**
 * @addtogroup  kern_thd
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
 * @notapi
 */
    struct thdL_ {
        struct esThdQ * q;                                                      /**< @brief Points to parent thread queue                   */
        struct esThd *  next;                                                   /**< @brief Next thread in linked list                      */
        struct esThd *  prev;                                                   /**< @brief Previous thread in linked list                  */
    }               thdL;                                                       /**< @brief Thread linked list                              */
    uint_fast8_t    prio;                                                       /**< @brief Thread priority level                           */
    uint_fast8_t    iprio;                                                      /**< @brief Initial Thread Priority level                   */
    uint_fast8_t    qCnt;                                                       /**< @brief Quantum counter                                 */
    uint_fast8_t    qRld;                                                       /**< @brief Quantum counter reload value                    */
#if   (1u== CFG_DBG_API_VALIDATION) || defined(__DOXYGEN__)
    portReg_T		signature;                                                  /**< @brief Thread structure signature                      */
#endif
};

/**@brief       Thread type
 */
typedef struct esThd esThd_T;

/**@brief       Stack type
 */
typedef portStck_T esStck_T;

/**@} *//*----------------------------------------------------------------*//**
 * @defgroup    kern_thdq Thread Queue
 * @brief       Thread Queue management services
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Thread Queue structure
 * @api
 */
struct esThdQ {

/**@brief       Priority Bit Map structure
 * @notapi
 */
    struct pbm_ {
#if   (CFG_SCHED_PRIO_LVL > PORT_DEF_DATA_WIDTH) || defined(__DOXYGEN__)
        portReg_T       bitGrp;                                                 /**< @brief Bit group indicator                             */
#endif
        portReg_T       bit[ES_DIV_ROUNDUP(CFG_SCHED_PRIO_LVL, PORT_DEF_DATA_WIDTH)];
                                                                                /**< @brief Bit priority indicator                          */
    }               prioOcc;                                                    /**< @brief Priority Occupancy                              */

/**@brief       Thread linked list sentinel structure
 * @notapi
 */
    struct thdLSent_ {
        struct esThd *  head;                                                   /**< @brief Points to the first thread in linked list.      */
        struct esThd *  next;                                                   /**< @brief Points to the next thread in linked list.       */
    }               grp[CFG_SCHED_PRIO_LVL];                                    /**< @brief Array of thread linked list sentinel structures.*/
#if   (1u== CFG_DBG_API_VALIDATION) || defined(__DOXYGEN__)
    portReg_T       signature;                                                  /**< @brief Thread Queue struct signature                   */
#endif
};

/**@brief       Thread queue type
 */
typedef struct esThdQ esThdQ_T;

/**@} *//*----------------------------------------------------------------*//**
 * @defgroup    kern_vtmr Virtual Timer
 * @brief       Virtual Timer management services
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Timer tick type
 */
#if   (2u == CFG_SYSTMR_TICK_TYPE) || defined(__DOXYGEN__)
typedef uint_fast32_t esTick_T;
#elif (1u== CFG_SYSTMR_TICK_TYPE)
typedef uint_fast16_t esTick_T;
#elif (0u == CFG_SYSTMR_TICK_TYPE)
typedef uint_fast8_t esTick_T;
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
    esTick_T        rtick;                                                      /**< @brief Relative tick value                             */
    void (* fn)(void *);                                                        /**< @brief Callback function pointer                       */
    void *          arg;                                                        /**< @brief Callback function argument                      */
#if   (1u== CFG_DBG_API_VALIDATION) || defined(__DOXYGEN__)
    portReg_T       signature;                                                  /**< @brief Timer structure signature                       */
#endif
};

/**@brief       Virtual Timer type
 */
typedef struct esVTmr esVTmr_T;

/**@} *//*----------------------------------------------------------------*//**
 * @defgroup    kern_ctrl Kernel control block
 * @brief       Low-level kernel information and status access
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Kernel state enumeration
 * @details     For more details see: @ref states
 * @api
 */
enum esKernState {
    ES_KERN_RUN         = 0x00u,                                                /**< Kernel is active                                       */
    ES_KERN_INTSRV_RUN  = 0x01u,                                                /**< Servicing an interrupt, return to ES_KERN_RUN state    */
    ES_KERN_LOCK        = 0x02u,                                                /**< Kernel is locked                                       */
    ES_KERN_INTSRV_LOCK = 0x03u,                                                /**< Servicing an interrupt, return to ES_KERN_LOCK state   */
    ES_KERN_SLEEP       = 0x06u,                                                /**< Kernel is sleeping                                     */
    ES_KERN_INIT        = 0x08u,                                                /**< Kernel is in initialization state                      */
    ES_KERN_INACTIVE    = 0x10u                                                 /**< Kernel data structures are not initialized             */
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
 * @notapi
 */
struct kernCtrl_ {
    struct esThd *      cthd;                                                   /**< @brief Pointer to the Current Thread                   */
    struct esThd *      pthd;                                                   /**< @brief Pointer to the Pending Thread to be switched    */
    enum esKernState    state;                                                  /**< @brief State of kernel                                 */
};

/**@} *//*----------------------------------------------------------------*//**
 * @defgroup    kern_lock Kernel lock
 * @brief       Kernel lock management
 * @details     These methods provide the most basic mechanism to protect
 *              concurrent access to a shared resource.
 *
 *              For more details see @ref critical_section.
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Kernel lock context type
 * @details     Variables declared using this type can hold current lock context
 *              which can be restored after a critical code section is exited.
 */
typedef portReg_T esLockCtx_T;

/** @} *//*-------------------------------------------------------------------*/

/*======================================================  GLOBAL VARIABLES  ==*/

/*------------------------------------------------------------------------*//**
 * @addtogroup  kern_ctrl
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Kernel control block
 * @note        This variable has Read-Only access rights for application.
 */
extern const volatile struct kernCtrl_ KernCtrl;

/**@} *//*--------------------------------------------------------------------*/

/*===================================================  FUNCTION PROTOTYPES  ==*/

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
void esKernInit(
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
PORT_C_NORETURN void esKernStart(
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
void esKernIsrEnterI(
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
void esKernIsrExitI(
    void);

/**@} *//*----------------------------------------------------------------*//**
 * @addtogroup  kern_lock
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Enter a critical code lock
 * @param       lockCtx
 *              Pointer to context variable where to store the current lock
 *              context.
 * @called
 * @fromapp
 * @fromthd
 * @schedno
 * @api
 */
void esKernLockIntEnter(
    esLockCtx_T *       lockCtx);

/**@brief       Exit a critical code lock
 * @param       lockCtx
 *              Context variable value
 * @details     Restores the lock context to state before the
 *              esKernLockIntEnter() was called.
 * @called
 * @fromapp
 * @fromthd
 * @schedmaybe
 * @api
 */
void esKernLockIntExit(
    esLockCtx_T         lockCtx);

/**@brief       Lock the scheduler
 * @pre         1) `The kernel state < ES_KERN_INIT`, see @ref states.
 * @called
 * @fromthd
 * @schedno
 * @iclass
 */
void esKernLockEnterI(
    void);

/**@brief       Unlock the scheduler
 * @pre         1) `The kernel state < ES_KERN_INIT`, see @ref states.
 * @pre         2) `gKernLockCnt > 0u`, current number of locks must be greater
 *                  than zero, in other words: each call to kernel lock function
 *                  must have its matching call to kernel unlock function.
 * @called
 * @fromthd
 * @schedmaybe
 * @iclass
 */
void esKernLockExitI(
    void);

/**@brief       Lock the scheduler
 * @pre         1) `The kernel state < ES_KERN_INIT`, see @ref states.
 * @called
 * @fromthd
 * @schedno
 * @api
 */
void esKernLockEnter(
    void);

/**@brief       Unlock the scheduler
 * @pre         1) `The kernel state < ES_KERN_INIT`, see @ref states.
 * @pre         2) `gKernLockCnt > 0u`, current number of locks must be greater
 *                  than zero, in other words: each call to kernel lock function
 *                  must have its matching call to kernel unlock function.
 * @called
 * @fromthd
 * @schedmaybe
 * @api
 */
void esKernLockExit(
    void);

/**@} *//*----------------------------------------------------------------*//**
 * @addtogroup  kern_thd
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Initialize the specified thread
 * @param       thd
 *              Thread: is a pointer to the thread structure, @ref esThd.
 *              The structure will be used as information container for the
 *              thread. It is assumed that storage for the `esThd` structure is
 *              allocated by the user code.
 * @param       fn
 *              Function: is a pointer to thread function. Thread function must
 *              have the following signature: `void thread (void * arg)`.
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
 *              sized stack. The stack type must be an array of @ref portStck.
 * @param       stckSize
 *              Stack Size: specifies the size of allocated stack memory. Size
 *              is expressed in bytes. Please see port documentation about
 *              minimal stack size. Usage of C unary operator `sizeof` is the
 *              recommended way of specifying stack size. Another way of
 *              specifying required stack size is through the usage of
 *              @ref ES_STCK_SIZE macro.
 * @param       prio
 *              Priority: is the priority of the thread. The higher the number,
 *              the higher the priority (the importance) of the thread. Several
 *              threads can have the same priority. Note that lowest (0) and
 *              highest (CFG_SCHED_PRIO_LVL - 1) levels are reserved for kernel
 *              threads only.
 * @pre         1) `The kernel state ES_KERN_INACTIVE`, see @ref states.
 * @pre         2) `thd != NULL`
 * @pre         3) `thd->signature != DEF_THD_CONTRACT_SIGNATURE`, the thread
 *                  structure can't be initialized more than once.
 * @pre         4) `fn != NULL`
 * @pre         5) `stckSize >= PORT_DEF_STCK_MINSIZE`, see
 *                  @ref PORT_DEF_STCK_MINSIZE.
 * @pre         6) `0 < prio < CFG_SCHED_PRIO_LVL - 1`, see
 *                  @ref CFG_SCHED_PRIO_LVL.
 * @post        1) `thd->signature == DEF_THD_CONTRACT_SIGNATURE`, each @ref esThd
 *                  structure will have valid signature after initialization.
 * @details     Threads must be created in order for kernel to recognize them as
 *              threads. Initialize a thread by calling esThdInit() and
 *              provide arguments specifying to kernel how the thread will be
 *              managed. Threads are always created in the @c ready-to-run state.
 *              Threads can be created either prior to the start of
 *              multi-threading (before calling esKernStart()), or by a running
 *              thread.
 * @called
 * @fromapp
 * @fromthd
 * @schedmaybe
 * @api
 */
void esThdInit(
    esThd_T *       thd,
    void (* fn)(void *),
    void *          arg,
    portStck_T *    stck,
    size_t          stckSize,
    uint8_t         prio);

/**@brief       Terminate the specified thread
 * @param       thd
 *              Thread: is a pointer to the thread structure, @ref esThd.
 * @pre         1) `The kernel state ES_KERN_INACTIVE`, see @ref states.
 * @pre         2) `thd != NULL`
 * @pre         3) `thd->signature == DEF_THD_CONTRACT_SIGNATURE`, the pointer must
 *                  point to a valid @ref esThd structure.
 * @pre         4) `(thd->thdL_.q == NULL) OR (thd->thdL_.q == gRdyQueue)`, thread
 *                  must be either in Ready Threads Queue or not be in any queue
 *                  (e.g. not waiting for a synchronization mechanism).
 * @post        1) `thd->signature == ~DEF_THD_CONTRACT_SIGNATURE`,  each
 *                  @ref esThd structure will have invalid signature after
 *                  termination.
 * @called
 * @fromapp
 * @fromthd
 * @schedmaybe
 * @api
 */
void esThdTerm(
    esThd_T *       thd);

/**@brief       Get the current thread ID
 * @return      Pointer to current thread ID structure @ref esThd.
 * @inline
 * @called
 * @fromapp
 * @fromthd
 * @fromisr
 * @schedno
 * @api
 */
static PORT_C_INLINE esThd_T * esThdGetId(
    void) {

    return (KernCtrl.cthd);
}

/**@brief       Get the priority of a thread
 * @param       thd
 *              Thread: is pointer to the thread structure, @ref esThd.
 * @return      The priority of the thread pointed by @c thd.
 * @inline
 * @called
 * @fromapp
 * @fromthd
 * @fromisr
 * @schedno
 * @api
 */
static PORT_C_INLINE uint8_t esThdGetPrio(
    esThd_T *       thd) {

    return (thd->prio);
}

/**@brief       Set the priority of a thread
 * @param       thd
 *              Thread: is pointer to the thread structure, @ref esThd.
 * @param       prio
 *              Priority: is new priority of the thread pointed by @c thd.
 * @pre         1) `The kernel state < ES_KERN_INACTIVE`, see @ref states.
 * @pre         2) `thd != NULL`
 * @pre         3) `thd->signature == DEF_THD_CONTRACT_SIGNATURE`, the pointer must
 *                  point to a valid @ref esThd structure.
 * @pre         4) `0 < prio < CFG_SCHED_PRIO_LVL - 1`, see
 *                  @ref CFG_SCHED_PRIO_LVL.
 * @called
 * @fromapp
 * @fromthd
 * @fromisr
 * @schedmaybe
 * @iclass
 */
void esThdSetPrioI(
    esThd_T *       thd,
    uint8_t         prio);

/**@} *//*----------------------------------------------------------------*//**
 * @addtogroup  kern_thdq
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Initialize Thread Queue
 * @param       thdQ
 *              Thread Queue: is a pointer to thread queue structure,
 *              @ref esThdQ.
 * @pre         1) `thdQ != NULL`
 * @pre         2) `thdQ->signature != DEF_THDQ_CONTRACT_SIGNATURE`, the thread
 *                  queue structure can't be initialized more than once.
 * @post        1) `thdQ->signature == DEF_THDQ_CONTRACT_SIGNATURE`, each
 *                  @ref esThdQ structure will have valid signature after
 *                  initialization.
 * @called
 * @fromapp
 * @fromthd
 * @fromisr
 * @schedno
 * @api
 */
void esThdQInit(
    esThdQ_T *      thdQ);

/**@brief       Terminate Thread Queue
 * @param       thdQ
 *              Thread Queue: is a pointer to thread queue structure,
 *              @ref esThdQ.
 * @pre         1) `thdQ != NULL`
 * @pre         2) `thdQ->signature == DEF_THDQ_CONTRACT_SIGNATURE`, the thread
 *                  queue structure must be already initialized.
 * @post        1) `thdQ->signature == ~DEF_THDQ_CONTRACT_SIGNATURE`, each
 *                  @ref esThdQ structure will have invalid signature after
 *                  termination.
 * @called
 * @fromapp
 * @fromthd
 * @fromisr
 * @schedno
 * @api
 */
void esThdQTerm(
    esThdQ_T *      thdQ);

/**@brief       Add a thread to the Thread Queue
 * @param       thdQ
 *              Thread Queue: is a pointer to thread queue structure,
 *              @ref esThdQ.
 * @param       thd
 *              Thread: is a pointer to the thread ID structure, @ref esThd.
 * @pre         1) `thdQ != NULL`
 * @pre         2) `thdQ->signature == DEF_THDQ_CONTRACT_SIGNATURE`, the pointer
 *                  must point to a valid @ref esThdQ structure.
 * @pre         3) `thd != NULL`
 * @pre         4) `thd->signature == DEF_THD_CONTRACT_SIGNATURE`, the pointer must
 *                  point to a valid @ref esThd structure.
 * @pre         5) `thd->thdL_.q == NULL`, thread must not be in any queue.
 * @details     This function adds a thread at the specified Thread Queue.
 * @called
 * @fromapp
 * @fromthd
 * @fromisr
 * @schedno
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
 *              Thread: is a pointer to the thread ID structure, @ref esThd.
 * @pre         1) `thd != NULL`
 * @pre         2) `thd->signature == DEF_THD_CONTRACT_SIGNATURE`, the pointer must
 *                  point to a valid @ref esThd structure.
 * @pre         3) `thdQ != NULL`
 * @pre         4) `thdQ->signature == DEF_THDQ_CONTRACT_SIGNATURE`, the pointer
 *                  must point to a valid @ref esThdQ structure.
 * @pre         5) `thd->thdL_.q == thdQ`, thread must be in the `thdQ` queue.
 * @called
 * @fromapp
 * @fromthd
 * @fromisr
 * @schedno
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
 * @pre         2) `thdQ->signature == DEF_THDQ_CONTRACT_SIGNATURE`, the pointer
 *                  must point to a valid @ref esThdQ structure.
 * @pre         3) `pbm_ != 0`, priority bit map must not be empty
 * @called
 * @fromapp
 * @fromthd
 * @fromisr
 * @schedno
 * @iclass
 */
esThd_T * esThdQFetchI(
    const esThdQ_T *    thdQ);

/**@brief       Fetch the next thread and rotate thread linked list
 * @param       thdQ
 *              Thread Queue: is a pointer to thread queue structure,
 *              @ref esThdQ. This is the thread queue to fetch from.
 * @param       prio
 *              Priority: is the priority level to fetch and rotate.
 * @return      Pointer to the next thread in queue.
 * @pre         1) `thdQ != NULL`
 * @pre         2) `thdQ->signature == DEF_THDQ_CONTRACT_SIGNATURE`, the pointer
 *                  must point to a valid @ref esThdQ structure.
 * @pre         3) `0 <= prio <= CFG_SCHED_PRIO_LVL`, see
 *                  @ref CFG_SCHED_PRIO_LVL.
 * @pre         4) `sentinel != NULL`, at least one thread must be in the
 *                  selected priority level
 * @called
 * @fromapp
 * @fromthd
 * @fromisr
 * @schedno
 * @iclass
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
 * @pre         2) `thdQ->signature == DEF_THDQ_CONTRACT_SIGNATURE`, the pointer
 *                  must point to a valid @ref esThdQ structure.
 * @called
 * @fromapp
 * @fromthd
 * @fromisr
 * @schedno
 * @api
 */
bool_T esThdQIsEmpty(
    const esThdQ_T *    thdQ);

/**@} *//*----------------------------------------------------------------*//**
 * @defgroup    kern_sched Scheduler notification and invocation
 * @brief       Low-level scheduler services
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Add thread `thd` to the ready thread list and notify the
 *              scheduler.
 * @param       thd
 *              Pointer to the initialized thread ID structure, @ref esThd.
 * @pre         1) `The kernel state < ES_KERN_INACTIVE`, see @ref states.
 * @pre         2) `thd != NULL`
 * @pre         3) `thd->signature == DEF_THD_CONTRACT_SIGNATURE`, the pointer must
 *                  point to a valid @ref esThd structure.
 * @pre         4) `thd->thdL_.q == NULL`, thread must not be in a queue.
 * @called
 * @fromapp
 * @fromthd
 * @fromisr
 * @schedno
 * @iclass
 */
void esSchedRdyAddI(
    esThd_T *       thd);

/**@brief       Remove thread `thd` from the ready thread list and notify the
 *              scheduler.
 * @param       thd
 *              Pointer to the initialized thread ID structure, @ref esThd.
 * @pre         1) `The kernel state < ES_KERN_INACTIVE`, see @ref states.
 * @pre         2) `thd != NULL`
 * @pre         3) `thd->signature == DEF_THD_CONTRACT_SIGNATURE`, the pointer must
 *                  point to a valid @ref esThd structure.
 * @pre         4) `thd->thdL_.q == &gRdyQueue`, thread must be in Ready Threads
 *                  queue.
 * @called
 * @fromapp
 * @fromthd
 * @fromisr
 * @schedno
 * @iclass
 */
void esSchedRdyRmI(
    esThd_T *       thd);

/**@brief       Force the scheduler invocation which will evaluate all ready
 *              threads and switch to ready thread with the highest priority
 * @pre         1) `The kernel state < ES_KERN_INACTIVE`, see @ref states.
 * @called
 * @fromthd
 * @schedmaybe
 * @iclass
 */
void esSchedYieldI(
    void);

/**@brief       Force the scheduler invocation which will evaluate all ready
 *              threads and switch to ready thread with the highest priority
 * @pre         1) `The kernel state < ES_KERN_INACTIVE`, see @ref states.
 * @called
 * @fromisr
 * @schedmaybe
 * @iclass
 */
void esSchedYieldIsrI(
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
void esVTmrInitI(
    esVTmr_T *      vTmr,
    esTick_T        tick,
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
    esTick_T        tick,
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
    esTick_T        tick);

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
esTick_T esSysTmrTickGet(
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
    esThd_T *       thd);

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
    esThd_T *       oldThd,
    esThd_T *       newThd);

/**@} *//*------------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//**@} *//**@} *//***********************************************
 * END of kernel.h
 ******************************************************************************/
#endif /* KERNEL_H__ */
