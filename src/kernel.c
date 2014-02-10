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
 * @author      Nenad Radulovic
 * @brief       Implementation of kernel port independent code
 * @addtogroup  kern
 *********************************************************************//** @{ */
/**@defgroup    kern_impl Implementation
 * @brief       Kernel port independent code implementation
 * @{ *//*--------------------------------------------------------------------*/

/*=========================================================  INCLUDE FILES  ==*/

#include "plat/critical.h"
#include "base/base.h"
#include "kernel/kernel.h"

/*=========================================================  LOCAL MACRO's  ==*/

/**@brief       Kernel state variable bit position which defines if the kernel
 *              is in interrupt servicing state.
 */
#define DEF_SCHED_STATE_INTSRV_MSK      (0x01u << 0)

/**@brief       Kernel state variable bit position which defines if the kernel
 *              is locked or not.
 */
#define DEF_SCHED_STATE_LOCK_MSK        (0x01u << 1)

/**@brief       Thread structure signature.
 * @details     The signature is used to confirm that a structure passed to a
 *              kernel function is indeed a esThread thread structure.
 */
#define DEF_THD_CONTRACT_SIGNATURE      ((esAtomic)0xfeedbeeful)

/**@brief       Thread Queue structure signature.
 * @details     The signature is used to confirm that a structure passed to a
 *              kernel function is indeed a esThreadQ thread queue structure.
 */
#define DEF_THDQ_CONTRACT_SIGNATURE     ((esAtomic)0xfeedbef0ul)

/**@brief       Timer structure signature.
 * @details     The signature is used to confirm that a structure passed to a
 *              timer function is indeed a esVTmr_T timer structure.
 */
#define DEF_VTMR_CONTRACT_SIGNATURE     ((esAtomic)0xfeedbef1ul)

/**@brief       DList macro: is the thread the first one in the list
 */
#define PQLIST_IS_ENTRY_FIRST(list, entry)                                       \
    ((entry) == (entry)->list.next)

/**@brief       DList macro: is the thread the last one in the list
 */
#define PQLIST_IS_ENTRY_LAST(list, entry)                                        \
    PQLIST_IS_ENTRY_FIRST(list, entry)

/**@brief       DList macro: is the thread single in the list
 */
#define PQLIST_IS_ENTRY_SINGLE(list, entry)                                      \
    PQLIST_IS_ENTRY_FIRST(list, entry)

/**@brief       DList macro: get the next entry
 */
#define PQLIST_ENTRY_NEXT(list, entry)                                           \
    (entry)->list.next

/**@brief       DList macro: initialize entry
 */
#define PQLIST_ENTRY_INIT(list, entry)                                           \
    do {                                                                        \
        (entry)->list.next = (entry);                                           \
        (entry)->list.prev = (entry);                                           \
    } while (0u)

/**@brief       DList macro: add new @c entry after @c current entry
 */
#define PQLIST_ENTRY_ADD_AFTER(list, current, entry)                             \
    do {                                                                        \
        (entry)->list.next = (current);                                         \
        (entry)->list.prev = (entry)->list.next->list.prev;                     \
        (entry)->list.next->list.prev = (entry);                                \
        (entry)->list.prev->list.next = (entry);                                \
    } while (0u)

/**@brief       DList macro: remove the @c entry from a list
 */
#define DLIST_ENTRY_RM(list, entry)                                             \
    do {                                                                        \
        (entry)->list.next->list.prev = (entry)->list.prev;                     \
        (entry)->list.prev->list.next = (entry)->list.next;                     \
    } while (0u)

/*======================================================  LOCAL DATA TYPES  ==*/

/*------------------------------------------------------------------------*//**
 * @name        System Timer
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Main System Timer structure
 * @note        1) Member `ptick` exists only if ADAPTIVE mode is selected. When
 *              this mode is selected then kernel supports more aggressive power
 *              savings.
 */
struct sysTmr {
    uint_fast16_t       vTmrArmed;                                              /**< @brief The number of armed virtual timers in system.   */
    uint_fast16_t       vTmrPend;                                               /**< @brief The number of pending timers for arming.        */
    esVTmrTick            ctick;                                                  /**< @brief Current system timer tick value.                */
#if   (1u == CFG_SYSTMR_ADAPTIVE_MODE) || defined(__DOXYGEN__)
    esVTmrTick            ptick;                                                  /**< @brief Pending ticks during the timer sleep mode.      */
#endif
};

/**@} *//*--------------------------------------------------------------------*/

/*=============================================  LOCAL FUNCTION PROTOTYPES  ==*/

/*------------------------------------------------------------------------*//**
 * @name        Priority Bit Map
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Initialize bitmap
 * @param       pbm
 *              Pointer to the bit map structure
 */
static PORT_C_INLINE void pbmInit(
    struct pbm_ *       pbm);

/**@brief       Set the bit corresponding to the priority argument
 * @param       pbm
 *              Pointer to the bit map structure
 * @param       priority
 *              Priority which will be marked as used
 */
static PORT_C_INLINE void pbmSet(
    struct pbm_ *       pbm,
    uint_fast8_t        prio);

/**@brief       Clear the bit corresponding to the priority argument
 * @param       pbm
 *              Pointer to the bit map structure
 * @param       priority
 *              Priority which will be marked as unused
 */
static PORT_C_INLINE void pbmClear(
    struct pbm_ *       pbm,
    uint_fast8_t        prio);

/**@brief       Get the highest priority set
 * @param       pbm
 *              Pointer to the bit map structure
 * @return      The number of the highest priority marked as used
 */
static PORT_C_INLINE uint_fast8_t pbmGetHighest(
    const struct pbm_ * pbm);

/**@brief       Is bit map empty?
 * @param       pbm
 *              Pointer to the bit map structure
 * @return      The status of the bit map
 *  @retval     true - bit map is empty
 *  @retval     false - there is at least one bit set
 */
static PORT_C_INLINE bool pbmIsEmpty(
    const struct pbm_ * pbm);


/**@} *//*----------------------------------------------------------------*//**
 * @name        Scheduler
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Initialize Ready Thread Queue structure @ref RdyQueue and
 *              Kernel control structure @ref kernCtrl_.
 */
static PORT_C_INLINE void schedInit(
    void);

/**@brief       Set the scheduler data structures for multi-threading
 * @details     This function is called just before multi-threading will start.
 */
static PORT_C_INLINE void schedStart(
    void);

/**@brief       Set the scheduler to sleep
 * @note        This function is used only when @ref CFG_SCHED_POWER_SAVE option
 *              is active.
 */
#if   (1u == CFG_SCHED_POWER_SAVE) || defined(__DOXYGEN__)
static PORT_C_INLINE void schedSleep(
    void);
#endif

/**@brief       Wake up the scheduler
 * @note        This function is used only when @ref CFG_SCHED_POWER_SAVE option
 *              is active.
 */
#if   (1u == CFG_SCHED_POWER_SAVE) || defined(__DOXYGEN__)
static PORT_C_INLINE void schedWakeUpI(
    void);
#endif

/**@brief       Initialize scheduler ready structure during the thread add
 *              operation
 * @param       thd
 *              Pointer to the thread currently being initialized.
 * @details     Function will initialize scheduler structures during the init
 *              phase of the kernel.
 */
static PORT_C_INLINE void schedRdyAddInitI(
    esThread *       thd);

/**@brief       Fetch and try to schedule the next thread of the same priority
 *              as the current thread
 */
static PORT_C_INLINE void schedQmNextI(
    void);

/**@brief       Do the Quantum (Round-Robin) scheduling
 */
static PORT_C_INLINE void schedQmI(
    void);

/**@} *//*----------------------------------------------------------------*//**
 * @name        System timer
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Initialize system timer hardware
 */
static PORT_C_INLINE void sysTmrInit(
    void);

/**@brief       Try to activate system timer
 * @note        This function is used only when @ref CFG_SYSTMR_ADAPTIVE_MODE
 *              option is active.
 */
#if   (1u == CFG_SYSTMR_ADAPTIVE_MODE) || defined(__DOXYGEN__)
static PORT_C_INLINE void sysTmrActivate(
    void);
#endif

/**@brief       Try to deactivate system timer
 * @note        This function is used only when @ref CFG_SYSTMR_ADAPTIVE_MODE
 *              option is active.
 */
#if   (1u == CFG_SYSTMR_ADAPTIVE_MODE) || defined(__DOXYGEN__)
static PORT_C_INLINE void sysTmrDeactivateI(
    void);
#endif

/**@} *//*----------------------------------------------------------------*//**
 * @name        Virtual Timer and Virtual Timer kernel thread
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Set up system timer for different tick period during sleeping
 * @param       ticks
 *              Number of ticks to sleep
 * @note        This function is used only when @ref CFG_SYSTMR_ADAPTIVE_MODE
 *              option is active.
 */
#if   (1u == CFG_SYSTMR_ADAPTIVE_MODE) || defined(__DOXYGEN__)
static PORT_C_INLINE void vTmrSleep(
    esVTmrTick            ticks);
#endif

/**@brief       Evaluate armed virtual timers
 */
static PORT_C_INLINE void vTmrEvaluateI(
    void);

/**@brief       Add a virtual timer into sorted list
 * @param       vTmr
 *              Virtual timer: pointer to virtual timer to add
 */
static void vTmrAddArmedS(
    esVTmr_T *          vTmr);

/**@brief       Import timers from pending list to armed list
 * @note        This function is used only when @ref CFG_SYSTMR_ADAPTIVE_MODE
 *              option is active.
 */
#if   (1u == CFG_SYSTMR_ADAPTIVE_MODE) || defined(__DOXYGEN__)
static PORT_C_INLINE void vTmrImportPendSleepI(
    void);
#endif

/**@brief       Import timers from pending list to armed list
 */
static void vTmrImportPend(
    void);

/**@brief       Initialization of Virtual Timer kernel thread
 */
static void kVTmrInit(
    void);

/**@brief       Virtual Timer thread code
 * @param       arg
 *              Argument: thread does not use argument
 * @details     This thread is responsible for virtual timer callback invocation
 *              and to import pending timers into armed linked list.
 */
static void kVTmr(
    void *              arg);

/**@} *//*----------------------------------------------------------------*//**
 * @name        Idle kernel thread
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Initialization of Idle thread
 */
static void kIdleInit(
    void);

/**@brief       Idle thread code
 * @param       arg
 *              Argument: thread does not use argument
 */
static void kIdle(
    void *              arg);

/**@} *//*----------------------------------------------------------------*//**
 * @name        Basic thread synchronization
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Post a signal to a thread which is waiting
 * @param       thd
 *              Pointer to thread which needs to be signaled
 */
void thdPost(
    esThread *           thd);

/**@brief       Wait for a signal
 */
void thdWait(
    void);

/**@} *//*--------------------------------------------------------------------*/

/*=======================================================  LOCAL VARIABLES  ==*/

/**@brief       Module identification info
 */
static ES_MODULE_INFO_CREATE("Kernel", ES_KERN_ID, "Nenad Radulovic");

/*------------------------------------------------------------------------*//**
 * @name        Scheduler
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Ready Thread queue
 */
static struct esThreadQ RdyQueue;

/**@} *//*----------------------------------------------------------------*//**
 * @name        System timer
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Main System Timer structure
 */
static struct sysTmr SysTmr = {
    0u,
    0u,
    0u,
#if   (1u == CFG_SYSTMR_ADAPTIVE_MODE)
    0u
#endif
};

/**@brief       List of virtual armed timers waiting to expire
 */
static struct esVTmr VTmrArmed = {
   {
        &VTmrArmed,
        &VTmrArmed,
        &VTmrArmed
   },

#if   (0u == CFG_SYSTMR_TICK_TYPE)
   UINT_FAST8_MAX,
#elif (1u == CFG_SYSTMR_TICK_TYPE)
   UINT_FAST16_MAX,
#elif (2u == CFG_SYSTMR_TICK_TYPE)
   UINT_FAST32_MAX,
#endif
   NULL,
   NULL,
#if   (1u == CONFIG_DEBUG_API_VALIDATION)
   DEF_VTMR_CONTRACT_SIGNATURE
#endif
};

/**@brief       Virtual timers pending to be inserted into waiting list
 */
static struct esVTmr VTmrPend = {
   {
        &VTmrPend,
        &VTmrPend,
        &VTmrPend
   },
   0u,
   NULL,
   NULL,
#if   (1u == CONFIG_DEBUG_API_VALIDATION)
   DEF_VTMR_CONTRACT_SIGNATURE
#endif
};

/**@brief       Virtual timer thread ID
 */
static struct esThread KVTmr;

/**@} *//*----------------------------------------------------------------*//**
 * @name        Idle kernel thread
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Idle thread ID
 */
static struct esThread KIdle;

/**@} *//*--------------------------------------------------------------------*/

/**@brief       Kernel Lock Counter
 */
static uint_fast8_t KernLockCnt;

/*======================================================  GLOBAL VARIABLES  ==*/

/**@brief       Kernel control initialization
 */
const volatile struct kernCtrl_ KernCtrl = {
    NULL,                                                                       /* No thread is currently executing                         */
    NULL,                                                                       /* No thread is pending                                     */
    ES_KERN_INACTIVE                                                            /* This is default kernel state before initialization       */
};

/*============================================  LOCAL FUNCTION DEFINITIONS  ==*/


/*--  Priority Bit Map  ------------------------------------------------------*/

static PORT_C_INLINE void pbmInit(
    struct pbm_ *       pbm) {

#if   (CFG_SCHED_PRIO_LVL > ES_CPU_DEF_DATA_WIDTH)
    uint_fast8_t        list;

    pbm->bitGroup = 0u;
    list         = ES_DIV_ROUNDUP(CFG_SCHED_PRIO_LVL, ES_CPU_DEF_DATA_WIDTH);

    while (0u != list) {
        list--;
        pbm->bit[list] = 0u;
    }
#else
    pbm->bit[0] = 0u;
#endif
}

static PORT_C_INLINE void pbmSet(
    struct pbm_ *       pbm,
    uint_fast8_t        prio) {

#if   (CFG_SCHED_PRIO_LVL > ES_CPU_DEF_DATA_WIDTH)
    uint_fast8_t        grpIndx;
    uint_fast8_t        bitIndx;

    bitIndx = priority &
        (~((uint_fast8_t)0u) >> (sizeof(priority) * 8u - ES_UINT8_LOG2(ES_CPU_DEF_DATA_WIDTH)));
    grpIndx = priority >> ES_UINT8_LOG2(ES_CPU_DEF_DATA_WIDTH);
    pbm->bitGroup |= ES_CPU_PWR2(grpIndx);
    pbm->bit[grpIndx] |= ES_CPU_PWR2(bitIndx);
#else
    pbm->bit[0] |= ES_CPU_PWR2(prio);
#endif
}

static PORT_C_INLINE void pbmClear(
    struct pbm_ *       pbm,
    uint_fast8_t        prio) {

#if   (CFG_SCHED_PRIO_LVL > ES_CPU_DEF_DATA_WIDTH)
    uint_fast8_t        grpIndx;
    uint_fast8_t        bitIndx;

    bitIndx = priority &
        (~((uint_fast8_t)0u) >> (sizeof(priority) * 8u - ES_UINT8_LOG2(ES_CPU_DEF_DATA_WIDTH)));
    grpIndx = priority >> ES_UINT8_LOG2(ES_CPU_DEF_DATA_WIDTH);
    pbm->bit[grpIndx] &= ~ES_CPU_PWR2(bitIndx);

    if (0u == pbm->bit[grpIndx]) {                                              /* Is this the last one bit cleared in this list?          */
        pbm->bitGroup &= ~ES_CPU_PWR2(grpIndx);                                   /* Yes: then clear bit list indicator, too.                */
    }
#else
    pbm->bit[0] &= ~ES_CPU_PWR2(prio);
#endif
}

static PORT_C_INLINE uint_fast8_t pbmGetHighest(
    const struct pbm_ * pbm) {

#if   (CFG_SCHED_PRIO_LVL > ES_CPU_DEF_DATA_WIDTH)
    uint_fast8_t        grpIndx;
    uint_fast8_t        bitIndx;

    grpIndx = ES_CPU_FLS(pbm->bitGroup);
    bitIndx = ES_CPU_FLS(pbm->bit[grpIndx]);

    return ((grpIndx << ES_UINT8_LOG2(ES_CPU_DEF_DATA_WIDTH)) | bitIndx);
#else
    uint_fast8_t        bitIndx;

    bitIndx = ES_CPU_FLS(pbm->bit[0]);

    return (bitIndx);
#endif
}

static PORT_C_INLINE bool pbmIsEmpty(
    const struct pbm_ * pbm) {

#if   (CFG_SCHED_PRIO_LVL > ES_CPU_DEF_DATA_WIDTH)
    bool              ret;

    if (0u == pbm->bitGroup) {
        ret = true;
    } else {
        ret = false;
    }

    return (ret);
#else
    bool              ret;

    if (0u == pbm->bit[0]) {
        ret = true;
    } else {
        ret = false;
    }

    return (ret);
#endif
}


/*--  Scheduler  -------------------------------------------------------------*/

static PORT_C_INLINE void schedInit(
    void) {

    esThdQInit(
        &RdyQueue);                                                             /* Initialize basic thread queue structure                  */
    ((volatile struct kernCtrl_ *)&KernCtrl)->state = ES_KERN_INIT;
}

static PORT_C_INLINE void schedStart(
    void) {

    esIntrCtx           intrCtx;
    esThread *           nthd;

    ES_CRITICAL_LOCK_ENTER(&intrCtx);
    nthd = esThdQFetchI(                                                        /* Get the highest priority thread                          */
        &RdyQueue);
    ((struct kernCtrl_ *)&KernCtrl)->cthd  = nthd;
    ((struct kernCtrl_ *)&KernCtrl)->pthd  = nthd;
    ((struct kernCtrl_ *)&KernCtrl)->state = ES_KERN_RUN;
    ES_CRITICAL_LOCK_EXIT(intrCtx);
}

#if   (1u == CFG_SCHED_POWER_SAVE) || defined(__DOXYGEN__)
static PORT_C_INLINE void schedSleep(
    void) {

    esIntrCtx           intrCtx;

    ES_CRITICAL_LOCK_ENTER(&intrCtx);
    esKernLockEnterI();
# if (1u == CFG_HOOK_PRE_IDLE)
    userPreIdle();
# endif
    ((struct kernCtrl_ *)&KernCtrl)->state = ES_KERN_SLEEP;
# if (1u == CFG_SYSTMR_ADAPTIVE_MODE)
    vTmrImportPendSleepI();                                                     /* Import any pending timers.                               */
    sysTmrDeactivateI();                                                        /* Evaluate timers and set system timer value for wake up.  */
# endif
    do {
# if (0u == CFG_DBG_ENABLE)
        /*
         * TODO: What to do here?
         */
#if 0
    PORT_CRITICAL_EXIT_SLEEP_ENTER();                                           /* Enter sleep state and wait for an interrupt.             */
#endif
# else
    ES_CRITICAL_LOCK_EXIT(intrCtx);
# endif
    ES_CRITICAL_LOCK_ENTER(&intrCtx);
    } while (KernCtrl.cthd == KernCtrl.pthd);
# if (1u == CFG_HOOK_POST_IDLE)
    userPostIdle();
# endif
    esKernLockExitI();
    ES_CRITICAL_LOCK_EXIT(intrCtx);
}
#endif

#if   (1u == CFG_SCHED_POWER_SAVE) || defined(__DOXYGEN__)
static PORT_C_INLINE void schedWakeUpI(
    void) {

    ((struct kernCtrl_ *)&KernCtrl)->state = ES_KERN_LOCK;
# if (1u == CFG_SYSTMR_ADAPTIVE_MODE)
    sysTmrActivate();                                                           /* Switch to normal system timer operation.                 */
# endif
}
#endif

static PORT_C_INLINE void schedRdyAddInitI(
    esThread *           thd) {

    if (KernCtrl.pthd == NULL) {
        ((struct kernCtrl_ *)&KernCtrl)->pthd = thd;
    }
}

static PORT_C_INLINE void schedQmNextI(
    void) {

    esThread * nthd;
    esThread * cthd;

    cthd = KernCtrl.cthd;
    nthd = esThdQFetchRotateI(                                                  /* Fetch the next thread and rotate this priority list     */
        &RdyQueue,
        cthd->prio);

    if (cthd == KernCtrl.pthd) {                                                /* If there is no any other thread pending for switching    */
        ((struct kernCtrl_ *)&KernCtrl)->pthd = nthd;                           /* Make the new thread pending                              */
    }
}

static PORT_C_INLINE void schedQmI(
    void) {

    if (ES_KERN_LOCK > KernCtrl.state) {                                        /* Round-Robin is not enabled in kernel LOCK state          */
        esThread * cthd;

        cthd = KernCtrl.cthd;                                                   /* Get the current thread                                   */

        if (!PQLIST_IS_ENTRY_SINGLE(thdL, cthd)) {
            cthd->quantumCounter--;                                                       /* Decrement current thread time quantum                    */

            if (0u == cthd->quantumCounter) {
                cthd->quantumCounter = cthd->quantumReload;                                        /* Reload thread time quantum                               */
                schedQmNextI();
            }
        }
    }
}


/*--  System timer  ----------------------------------------------------------*/

static PORT_C_INLINE void sysTmrInit(
    void) {

    ES_SYSTIMER_INIT(ES_SYSTIMER_ONE_TICK);
    ES_SYSTIMER_ENABLE();
    ES_SYSTIMER_ISR_ENABLE();
}

#if   (1u == CFG_SYSTMR_ADAPTIVE_MODE) || defined(__DOXYGEN__)
static PORT_C_INLINE void sysTmrActivate(
    void) {

    if (0u == SysTmr.ptick) {                                                   /* Normal wake up.                                          */

        if (0u != SysTmr.vTmrArmed) {                                           /* System timer was enabled during sleep.                   */
            portSysTmrReg_T tmrVal;

            tmrVal = ES_SYSTIMER_GET_RVAL() - ES_SYSTIMER_GET_CVAL();
            tmrVal = ES_SYSTIMER_ONE_TICK - tmrVal;
            ES_SYSTIMER_RELOAD(tmrVal);
        } else {                                                                /* System timer was disabled during sleep.                  */
            ES_SYSTIMER_RELOAD(ES_SYSTIMER_ONE_TICK);                          /* Reload macro will also re-enable the timer               */
            ES_SYSTIMER_ISR_ENABLE();
        }
    } else {                                                                    /* Preempted wake up, system timer was enabled during sleep.*/
        esVTmr_T *  vTmr;
        esVTmrTick    ticks;
        portSysTmrReg_T tmrVal;

        tmrVal = ES_SYSTIMER_GET_RVAL() - ES_SYSTIMER_GET_CVAL();
        ticks = tmrVal / ES_SYSTIMER_ONE_TICK;
        tmrVal -= (ES_SYSTIMER_ONE_TICK * ticks);
        tmrVal = ES_SYSTIMER_ONE_TICK - tmrVal;

        if (PORT_DEF_SYSTMR_WAKEUP_TH_VAL > tmrVal) {
            ES_SYSTIMER_RELOAD(tmrVal);
        } else {
            ES_SYSTIMER_RELOAD(tmrVal + ES_SYSTIMER_ONE_TICK);
        }
        vTmr = PQLIST_ENTRY_NEXT(tmrL_, &VTmrArmed);
        vTmr->rtick -= ticks;
        SysTmr.ctick += ticks;
        SysTmr.ptick = 0u;
    }
}
#endif

#if   (1u == CFG_SYSTMR_ADAPTIVE_MODE) || defined(__DOXYGEN__)
static PORT_C_INLINE void sysTmrDeactivateI(
    void) {

    if (0u != SysTmr.vTmrArmed) {                                               /* There is an armed timer: put system timer to sleep sleep.*/
        esVTmr_T * vTmr;

        vTmr = PQLIST_ENTRY_NEXT(tmrL_, &VTmrArmed);
        vTmrSleep(
            vTmr->rtick);
    } else {                                                                    /* No virtual timer is armed: set system timer to disabled. */
        ES_SYSTIMER_ISR_DISABLE();
        ES_SYSTIMER_DISABLE();
    }
}
#endif


/*--  Virtual Timer and Virtual Timer kernel thread --------------------------*/

#if   (1u == CFG_SYSTMR_ADAPTIVE_MODE) || defined(__DOXYGEN__)
static PORT_C_INLINE void vTmrSleep(
    esVTmrTick            ticks) {

    portSysTmrReg_T sysTmrVal;

    if (ES_SYSTIMER_MAX_TICKS < ticks) {                                    /* Limit the number of ticks according to hardware maximum  */
        ticks = ES_SYSTIMER_MAX_TICKS;                                      /* specification.                                           */
    }
    SysTmr.ptick = ticks;

    if (0u != ticks) {
        sysTmrVal = ES_SYSTIMER_ONE_TICK * (ticks - 1u);
        sysTmrVal += ES_SYSTIMER_GET_CVAL() % ES_SYSTIMER_ONE_TICK;         /* Add the remaining time of current tick period.           */

        if (ES_SYSTIMER_GET_RVAL() != sysTmrVal) {
            ES_SYSTIMER_RELOAD(sysTmrVal);
        }
    }
}
#endif

static PORT_C_INLINE void vTmrEvaluateI(
    void) {
    ++SysTmr.ctick;

    if (0u != SysTmr.vTmrArmed) {                                               /* There is an armed timer waiting.                         */
        esVTmr_T * vTmr;

        vTmr = PQLIST_ENTRY_NEXT(tmrL, &VTmrArmed);

#if   (0u == CFG_SYSTMR_ADAPTIVE_MODE)
        --vTmr->rtick;
#elif (1u == CFG_SYSTMR_ADAPTIVE_MODE)
        if (0u == SysTmr.ptick) {                                               /* Normal system tick.                                      */

            if (ES_SYSTIMER_ONE_TICK != ES_SYSTIMER_GET_RVAL()) {           /* If system timer is in adaptive mode switch it to fixed   */
                ES_SYSTIMER_RELOAD(ES_SYSTIMER_ONE_TICK);                      /* mode.                                                    */
            }
            --vTmr->rtick;
        } else {                                                                /* Low power tick.                                          */
            SysTmr.ctick += SysTmr.ptick - 1u;
            vTmr->rtick -= SysTmr.ptick;                                        /* Subtract pending ticks from current timer.               */
            vTmrSleep(                                                          /* time period.                                             */
                vTmr->rtick);
        }
#endif

        if (0u == vTmr->rtick) {                                                /* A timer has expired, start kVTmr thread.                 */
            esSchedRdyAddI(
                &KVTmr);
        }
    }

    if (0u != SysTmr.vTmrPend) {                                                /* There is a timer pending, start kVTmr thread.            */

        if (NULL == KVTmr.thdL.q) {
            esSchedRdyAddI(
                &KVTmr);
        }
    }
}

/* 1)       This function requires locked System mode
 */
static void vTmrAddArmedS(
    esVTmr_T *          vTmr) {

    esVTmr_T *          tmp;
    esVTmrTick            tick;

    vTmr->tmrL.q = &VTmrArmed;
    tmp = PQLIST_ENTRY_NEXT(tmrL, &VTmrArmed);
    tick = vTmr->rtick;

    while (tmp->rtick <= tick) {
        tick -= tmp->rtick;
        tmp = PQLIST_ENTRY_NEXT(tmrL, tmp);
    }
    vTmr->rtick = tick;
    PQLIST_ENTRY_ADD_AFTER(tmrL, tmp, vTmr);

    if (&VTmrArmed != tmp) {
        tmp->rtick -= vTmr->rtick;
    }
}

#if   (1u == CFG_SYSTMR_ADAPTIVE_MODE) || defined(__DOXYGEN__)
static PORT_C_INLINE void vTmrImportPendSleepI(
    void) {

    while (0u != SysTmr.vTmrPend) {
        esVTmr_T * tmr;

        --SysTmr.vTmrPend;
        ++SysTmr.vTmrArmed;
        tmr = PQLIST_ENTRY_NEXT(tmrL_, &VTmrPend);
        DLIST_ENTRY_RM(tmrL_, tmr);
        vTmrAddArmedS(
            tmr);
    }
}
#endif

/* 1)       Since an system timer interrupt has already occurred one tick period
 *          must be subtracted from imported timers.
 */
static void vTmrImportPend(
    void) {

    esIntrCtx           intrCtx;

    ES_CRITICAL_LOCK_ENTER(&intrCtx);
    esKernLockEnterI();

    while (0u != SysTmr.vTmrPend) {
        esVTmr_T * tmr;

        --SysTmr.vTmrPend;
        ++SysTmr.vTmrArmed;
        tmr = PQLIST_ENTRY_NEXT(tmrL, &VTmrPend);
        DLIST_ENTRY_RM(tmrL, tmr);
        ES_CRITICAL_LOCK_EXIT(intrCtx);
        --tmr->rtick;                                                           /* Timer thread requires one tick less than original value. */
        vTmrAddArmedS(
            tmr);
        ES_CRITICAL_LOCK_ENTER(&intrCtx);
    }
    esKernLockExitI();
    ES_CRITICAL_LOCK_EXIT(intrCtx);
}

/* 1)       Kernel Virtual Timer thread must have the highest priority available.
 */
static void kVTmrInit(
    void) {

    static esThreadStack kVTmrStck[ES_STACK_SIZE(PORT_DEF_KVTMR_STCK_SIZE)];        /* Virtual timer kernel thread stack.                       */

    esThdInit(
        &KVTmr,
        kVTmr,
        NULL,
        kVTmrStck,
        sizeof(kVTmrStck),
        CFG_SCHED_PRIO_LVL - 1u);
}

/* 1)       This thread is just waiting continuously on thread semaphore and
 *          will execute virtual timers callback functions if there are any
 *          available. After that it will import pending virtual timers into
 *          armed linked list.
 */
static void kVTmr(
    void *              arg) {

    (void)arg;

    while (true) {

        thdWait();

        if (0u != SysTmr.vTmrArmed) {                                           /* There is at least one armed timer.                       */
            esVTmr_T * tmr;

            tmr = PQLIST_ENTRY_NEXT(tmrL, &VTmrArmed);

            while (0u == tmr->rtick) {
                esVTmr_T * tmpTmr;

                --SysTmr.vTmrArmed;
                DLIST_ENTRY_RM(tmrL, tmr);
                tmpTmr = tmr;
                ES_DEBUG_API_OBLIGATION(tmr->signature = ~DEF_VTMR_CONTRACT_SIGNATURE);
                tmr = PQLIST_ENTRY_NEXT(tmrL, &VTmrArmed);
                (* tmpTmr->fn)(tmpTmr->arg);
            }
        }
        vTmrImportPend();                                                       /* Import pending timers if there are any.                  */
    }
}


/*--  Idle kernel thread  ----------------------------------------------------*/

/* 1)       Kernel Idle thread must have the lowest priority.
 */
static void kIdleInit(
    void) {

    static esThreadStack kIdleStck[ES_STACK_SIZE(PORT_DEF_KIDLE_STCK_SIZE)];        /* Idle kernel thread stack.                                */

    esThdInit(
        &KIdle,
        kIdle,
        NULL,
        kIdleStck,
        sizeof(kIdleStck),
        0u);
}

/* 1)       Idle thread must be the only thread at this priority level.
 */
static void kIdle(
    void *              arg){

    (void)arg;

    while (true) {
#if   (1u == CFG_SCHED_POWER_SAVE)
        schedSleep();
#endif
    }
}


/*--  Basic thread synchronization  ------------------------------------------*/

/* 1)       Since this function can be called multiple times with the same
 *          thread then it needs to check if the thread is not already added in
 *          a queue.
 */
void thdPost(
    esThread *           thd) {

    esIntrCtx           intrCtx;

    ES_CRITICAL_LOCK_ENTER(&intrCtx);
    if (NULL == thd->thdL.q) {
        esSchedRdyAddI(
            thd);
        esSchedYieldI();
    }
    ES_CRITICAL_LOCK_EXIT(intrCtx);
}

void thdWait(
    void) {

    esIntrCtx           intrCtx;

    ES_CRITICAL_LOCK_ENTER(&intrCtx);
    esSchedRdyRmI(
        esThdGetId());
    esSchedYieldI();
    ES_CRITICAL_LOCK_EXIT(intrCtx);
}

/*===================================  GLOBAL PRIVATE FUNCTION DEFINITIONS  ==*/
/*====================================  GLOBAL PUBLIC FUNCTION DEFINITIONS  ==*/


/*--  General kernel functions  ----------------------------------------------*/

void esKernInit(
    void) {

    ES_DEBUG_API_REQUIRE(ES_DEBUG_USAGE, ES_KERN_INACTIVE == KernCtrl.state);

#if   (1u == CFG_HOOK_PRE_KERN_INIT)
    userPreKernInit();
#endif
    ES_INTR_DISABLE();
    PORT_KCORE_INIT_EARLY();
    sysTmrInit();
    schedInit();
    kIdleInit();
    kVTmrInit();
    PORT_KCORE_INIT();
#if   (1u == CFG_HOOK_POST_KERN_INIT)
    userPostKernInit();
#endif
}

/* 1)       Since this function will never return it is marked with `noreturn`
 *          attribute to allow for compiler optimizations.
 */
PORT_C_NORETURN void esKernStart(
    void) {

    ES_DEBUG_API_REQUIRE(ES_DEBUG_USAGE, ES_KERN_INIT == KernCtrl.state);

#if   (1u == CFG_HOOK_PRE_KERN_START)
    userPreKernStart();
#endif
    PORT_KCORE_INIT_LATE();
    schedStart();                                                               /* Initialize scheduler data structures for multi-threading */
    PORT_CTX_SW_START();                                                        /* Start the first thread                                   */

    while (true);                                                               /* Prevent compiler `function does return` warnings.        */
}

void esKernSysTmr(
    void) {

    esIntrCtx           intrCtx;

    ES_DEBUG_API_REQUIRE(ES_DEBUG_USAGE, KernCtrl.cthd != &KVTmr);

#if   (1u == CFG_HOOK_PRE_SYSTMR_EVENT)
    userPreSysTmr();
#endif
    ES_CRITICAL_LOCK_ENTER(&intrCtx);
    vTmrEvaluateI();
    schedQmI();
    ES_CRITICAL_LOCK_EXIT(intrCtx);
}

void esKernIsrEnterI(
    void) {

    ES_DEBUG_API_REQUIRE(ES_DEBUG_USAGE, ES_KERN_INIT > KernCtrl.state);

    PORT_ISR_ENTER();
    ((struct kernCtrl_ *)&KernCtrl)->state |= DEF_SCHED_STATE_INTSRV_MSK;
}

void esKernIsrExitI(
    void) {

    ES_DEBUG_API_REQUIRE(ES_DEBUG_USAGE, ES_KERN_INIT > KernCtrl.state);

    PORT_ISR_EXIT();

    if (true == PORT_ISR_IS_LAST()) {
        ((struct kernCtrl_ *)&KernCtrl)->state &= ~DEF_SCHED_STATE_INTSRV_MSK;
        esSchedYieldIsrI();
    }
}


/*--  Critical code section locking management  ------------------------------*/

void esKernLockIntEnter(
    esIntrCtx *       lockCtx) {

    ES_CRITICAL_LOCK_ENTER(lockCtx);
    esKernLockEnterI();
}

void esKernLockIntExit(
    esIntrCtx         lockCtx) {

    esKernLockExitI();
    ES_CRITICAL_LOCK_EXIT(lockCtx);
}

void esKernLockEnterI(
    void) {

    ES_DEBUG_API_REQUIRE(ES_DEBUG_USAGE, ES_KERN_INIT > KernCtrl.state);

    ((struct kernCtrl_ *)&KernCtrl)->state |= DEF_SCHED_STATE_LOCK_MSK;
    ++KernLockCnt;
}

void esKernLockExitI(
    void) {

    ES_DEBUG_API_REQUIRE(ES_DEBUG_USAGE, ES_KERN_LOCK == KernCtrl.state);
    ES_DEBUG_API_REQUIRE(ES_DEBUG_USAGE, 0u != KernLockCnt);

    --KernLockCnt;

    if (0u == KernLockCnt) {
        ((struct kernCtrl_ *)&KernCtrl)->state &= ~DEF_SCHED_STATE_LOCK_MSK;
        esSchedYieldI();
    }
}

void esKernLockEnter(
    void) {

    esIntrCtx           intrCtx;

    ES_CRITICAL_LOCK_ENTER(&intrCtx);
    esKernLockEnterI();
    ES_CRITICAL_LOCK_EXIT(intrCtx);
}

void esKernLockExit(
    void) {

    esIntrCtx           intrCtx;

    ES_CRITICAL_LOCK_ENTER(&intrCtx);
    esKernLockExitI();
    ES_CRITICAL_LOCK_EXIT(intrCtx);
}


/*--  Thread management  -----------------------------------------------------*/

void esThdInit(
    esThread *           thd,
    void (* fn)(void *),
    void *              arg,
    esThreadStack *        stck,
    size_t              stckSize,
    uint8_t             prio) {

    esIntrCtx           intrCtx;

    ES_DEBUG_API_REQUIRE(ES_DEBUG_USAGE, ES_KERN_INACTIVE > KernCtrl.state);
    ES_DEBUG_API_REQUIRE(ES_DEBUG_POINTER, NULL != thd);
    ES_DEBUG_API_REQUIRE(ES_DEBUG_OBJECT, DEF_THD_CONTRACT_SIGNATURE != thd->signature);
    ES_DEBUG_API_REQUIRE(ES_DEBUG_POINTER, NULL != fn);
    ES_DEBUG_API_REQUIRE(ES_DEBUG_POINTER, NULL != stck);
    ES_DEBUG_API_REQUIRE(ES_DEBUG_RANGE, PORT_STACK_MINSIZE <= (stckSize * sizeof(esCpuReg)));
    ES_DEBUG_API_REQUIRE(ES_DEBUG_RANGE, CFG_SCHED_PRIO_LVL > prio);
    ES_DEBUG_API_REQUIRE(ES_DEBUG_RANGE, ((&KVTmr != thd) && ((CFG_SCHED_PRIO_LVL - 1u) > prio)) || (&KVTmr == thd));
    ES_DEBUG_API_REQUIRE(ES_DEBUG_RANGE, ((&KIdle != thd) && (0u < prio)) || (&KIdle == thd));

    thd->stack   = PORT_CTX_INIT(stck, stckSize, fn, arg);                       /* Make a fake thread stack.                                */
    thd->thdL.q = NULL;                                                         /* This thread is not in any thread queue.                  */
    PQLIST_ENTRY_INIT(thdL, thd);
    thd->prio   = prio;                                                         /* Set the priority.                                        */
    thd->ipriority  = prio;                                                         /* This is constant priority, it never changes.             */
    thd->quantumCounter   = CFG_SCHED_TIME_QUANTUM;
    thd->quantumReload   = CFG_SCHED_TIME_QUANTUM;

    ES_DEBUG_API_OBLIGATION(thd->signature = DEF_THD_CONTRACT_SIGNATURE);             /* Make thread structure valid.                             */

    ES_CRITICAL_LOCK_ENTER(&intrCtx);
    schedRdyAddInitI(
        thd);                                                                   /* Initialize thread before adding it to Ready Thread queue.*/
    esSchedRdyAddI(
        thd);                                                                   /* Add the thread to Ready Thread queue.                    */
    esSchedYieldI();                                                            /* Invoke the scheduler.                                    */
    ES_CRITICAL_LOCK_EXIT(intrCtx);

#if   (1u == CFG_HOOK_POST_THD_INIT)
    userPostThdInit();
#endif
}

void esThdTerm(
    esThread *           thd) {

    esIntrCtx           intCtx;

    ES_DEBUG_API_REQUIRE(ES_DEBUG_USAGE, ES_KERN_INACTIVE > KernCtrl.state);
    ES_DEBUG_API_REQUIRE(ES_DEBUG_POINTER, NULL != thd);
    ES_DEBUG_API_REQUIRE(ES_DEBUG_OBJECT, DEF_THD_CONTRACT_SIGNATURE == thd->signature);
    ES_DEBUG_API_REQUIRE(ES_DEBUG_POINTER, (NULL == thd->thdL.q) || (&RdyQueue == thd->thdL.q));

#if   (1u == CFG_HOOK_PRE_THD_TERM)
    userPreThdTerm();
#endif
    ES_CRITICAL_LOCK_ENTER(&intCtx);

    if (&RdyQueue == thd->thdL.q) {
        esSchedRdyRmI(
            thd);
    } else if (NULL != thd->thdL.q) {
        esThdQRmI(
            thd->thdL.q,
            thd);
    }

    ES_DEBUG_API_OBLIGATION(thd->signature = ~DEF_THD_CONTRACT_SIGNATURE);            /* Mark the thread ID structure as invalid.                 */

    esSchedYieldI();
    ES_CRITICAL_LOCK_EXIT(intCtx);
}

void esThdSetPrioI(
    esThread *           thd,
    uint8_t             prio) {

    ES_DEBUG_API_REQUIRE(ES_DEBUG_USAGE, ES_KERN_INACTIVE > KernCtrl.state);
    ES_DEBUG_API_REQUIRE(ES_DEBUG_POINTER, NULL != thd);
    ES_DEBUG_API_REQUIRE(ES_DEBUG_OBJECT, DEF_THD_CONTRACT_SIGNATURE == thd->signature);
    ES_DEBUG_API_REQUIRE(ES_DEBUG_RANGE, CFG_SCHED_PRIO_LVL >= prio);

    if (NULL == thd->thdL.q) {                                                  /* Is thread inserted in any queue?                         */
        thd->prio = prio;                                                       /* Just change it's priority value.                             */
    } else {
        esThdQRmI(
            thd->thdL.q,
            thd);
        thd->prio = prio;
        esThdQAddI(
            thd->thdL.q,
            thd);

        if (&RdyQueue == thd->thdL.q) {                                         /* Is thread in ready thread queue?                         */

            if (thd->prio > KernCtrl.pthd->prio) {                              /* If new priority is higher than the current priority notify the   */
                ((struct kernCtrl_ *)&KernCtrl)->pthd = thd;                    /* scheduler about new thread.                              */
            } else {
                ((struct kernCtrl_ *)&KernCtrl)->pthd = esThdQFetchI(
                    &RdyQueue);
            }
        }
    }
}


/*--  Thread Queue management  -----------------------------------------------*/

void esThdQInit(
    esThreadQ *          thdQ) {

    uint_fast8_t        grp;

    ES_DEBUG_API_REQUIRE(ES_DEBUG_POINTER, NULL != thdQ);
    ES_DEBUG_API_REQUIRE(ES_DEBUG_OBJECT, DEF_THDQ_CONTRACT_SIGNATURE != thdQ->signature);

    esPqInit(&thdQ->pq);
    ES_DEBUG_API_OBLIGATION(thdQ->signature = DEF_THDQ_CONTRACT_SIGNATURE);
}

/* 1)       When API validation is not used then this function will become empty.
 */
void esThdQTerm(
    esThreadQ *          thdQ) {

    ES_DEBUG_API_REQUIRE(ES_DEBUG_POINTER, NULL != thdQ);
    ES_DEBUG_API_REQUIRE(ES_DEBUG_OBJECT, DEF_THDQ_CONTRACT_SIGNATURE == thdQ->signature);

    ES_DEBUG_API_OBLIGATION(thdQ->signature = ~DEF_THDQ_CONTRACT_SIGNATURE);

#if   (0u == CONFIG_DEBUG_API_VALIDATION)
    (void)thdQ;                                                                 /* Prevent compiler warning about unused argument.          */
#endif
}

void esThdQAddI(
    esThreadQ *          thdQ,
    esThread *           thd) {

    struct thdLSent_ *  sentinel;

    ES_DEBUG_API_REQUIRE(ES_DEBUG_POINTER, NULL != thdQ);
    ES_DEBUG_API_REQUIRE(ES_DEBUG_OBJECT, DEF_THDQ_CONTRACT_SIGNATURE == thdQ->signature);
    ES_DEBUG_API_REQUIRE(ES_DEBUG_POINTER, NULL != thd);
    ES_DEBUG_API_REQUIRE(ES_DEBUG_OBJECT, DEF_THD_CONTRACT_SIGNATURE == thd->signature);

    esPqAdd(&thdQ->pq, &thd->pqElem);
}

void esThdQRmI(
    esThreadQ *          thdQ,
    esThread *           thd) {

    struct thdLSent_ *  sentinel;

    ES_DEBUG_API_REQUIRE(ES_DEBUG_POINTER, NULL != thd);
    ES_DEBUG_API_REQUIRE(ES_DEBUG_OBJECT, DEF_THD_CONTRACT_SIGNATURE == thd->signature);
    ES_DEBUG_API_REQUIRE(ES_DEBUG_POINTER, NULL != thdQ);
    ES_DEBUG_API_REQUIRE(ES_DEBUG_OBJECT, DEF_THDQ_CONTRACT_SIGNATURE == thdQ->signature);

    esPqRm(&thd->pqElem);
}

esThread * esThdQFetchI(
    const esThreadQ *    thdQ) {

    struct thdLSent_ *  sentinel;
    uint_fast8_t        prio;

    ES_DEBUG_API_REQUIRE(ES_DEBUG_POINTER, NULL != thdQ);
    ES_DEBUG_API_REQUIRE(ES_DEBUG_OBJECT, DEF_THDQ_CONTRACT_SIGNATURE == thdQ->signature);
    ES_DEBUG_API_REQUIRE(ES_DEBUG_USAGE, false == pbmIsEmpty(&thdQ->prioOcc));

    prio = pbmGetHighest(
        &thdQ->prioOcc);                                                        /* Get the highest priority ready to run.                   */
    sentinel = (struct thdLSent_ *)&(thdQ->grp[prio]);                          /* Get the Group Head pointer for that priority.            */
                                                                                /* The type cast is needed to avoid compiler warnings.      */
    ES_DBG_API_ENSURE(ES_DEBUG_OBJECT, DEF_THD_CONTRACT_SIGNATURE == sentinel->next->signature);

    return (sentinel->next);
}

esThread * esThdQFetchRotateI(
    esThreadQ *          thdQ,
    uint_fast8_t        prio) {

    struct thdLSent_ *  sentinel;

    ES_DEBUG_API_REQUIRE(ES_DEBUG_POINTER, NULL != thdQ);
    ES_DEBUG_API_REQUIRE(ES_DEBUG_OBJECT, DEF_THDQ_CONTRACT_SIGNATURE == thdQ->signature);
    ES_DEBUG_API_REQUIRE(ES_DEBUG_RANGE, CFG_SCHED_PRIO_LVL >= prio);
    ES_DEBUG_API_REQUIRE(ES_DEBUG_POINTER, NULL != thdQ->grp[prio].next);

    sentinel = &(thdQ->grp[prio]);                                              /* Get the Group Head pointer from thread priority.         */
    sentinel->next = PQLIST_ENTRY_NEXT(thdL, sentinel->next);

    ES_DBG_API_ENSURE(ES_DEBUG_OBJECT, DEF_THD_CONTRACT_SIGNATURE == sentinel->next->signature);

    return (sentinel->next);
}

bool esThdQIsEmpty(
    const esThreadQ *    thdQ) {

    bool              ret;

    ES_DEBUG_API_REQUIRE(ES_DEBUG_POINTER, NULL != thdQ);
    ES_DEBUG_API_REQUIRE(ES_DEBUG_OBJECT, DEF_THDQ_CONTRACT_SIGNATURE == thdQ->signature);

    ret = pbmIsEmpty(
        &thdQ->prioOcc);

    return (ret);
}


/*--  Scheduler notification and invocation  ---------------------------------*/

void esSchedRdyAddI(
    esThread *           thd) {

    ES_DEBUG_API_REQUIRE(ES_DEBUG_USAGE, ES_KERN_INACTIVE > KernCtrl.state);
    ES_DEBUG_API_REQUIRE(ES_DEBUG_POINTER, NULL != thd);
    ES_DEBUG_API_REQUIRE(ES_DEBUG_OBJECT, DEF_THD_CONTRACT_SIGNATURE == thd->signature);
    ES_DEBUG_API_REQUIRE(ES_DEBUG_POINTER, NULL == thd->thdL.q);

    esThdQAddI(
        &RdyQueue,
        thd);

    if (thd->prio > KernCtrl.pthd->prio) {
        ((struct kernCtrl_ *)&KernCtrl)->pthd = thd;
    }
}

/* 1)       If this function is removing currently executed thread or the
 *          pending thread then the scheduler will be invoked to get new highest
 *          priority thread.
 */
void esSchedRdyRmI(
    esThread *           thd) {

    ES_DEBUG_API_REQUIRE(ES_DEBUG_USAGE, ES_KERN_INACTIVE > KernCtrl.state);
    ES_DEBUG_API_REQUIRE(ES_DEBUG_POINTER, NULL != thd);
    ES_DEBUG_API_REQUIRE(ES_DEBUG_OBJECT, DEF_THD_CONTRACT_SIGNATURE == thd->signature);
    ES_DEBUG_API_REQUIRE(ES_DEBUG_USAGE, &RdyQueue == thd->thdL.q);

    esThdQRmI(
        &RdyQueue,
        thd);

    if ((KernCtrl.cthd == thd) || (KernCtrl.pthd == thd)) {
        ((struct kernCtrl_ *)&KernCtrl)->pthd = esThdQFetchI(
            &RdyQueue);                                                         /* Get new highest priority thread.                         */
    }
}

void esSchedYieldI(
    void) {

    ES_DEBUG_API_REQUIRE(ES_DEBUG_USAGE, ES_KERN_INACTIVE > KernCtrl.state);

    if (KernCtrl.cthd != KernCtrl.pthd) {                                       /* Context switching is needed only when cthd and pthd are  */
                                                                                /* different.                                               */
        if (ES_KERN_RUN == KernCtrl.state) {

#if   (1u == CFG_HOOK_PRE_CTX_SW)
            userPreCtxSw(
                KernCtrl.cthd,
                newThd);
#endif
            PORT_CTX_SW();
        }
    }
}

/* 1)       This function is similar to esSchedYieldI() except it calls
 *          context switching macro for ISR and can wake up scheduler after
 *          idle sleep.
 */
void esSchedYieldIsrI(
    void) {

    ES_DEBUG_API_REQUIRE(ES_DEBUG_USAGE, ES_KERN_INACTIVE > KernCtrl.state);

    if (KernCtrl.cthd != KernCtrl.pthd) {

        if (ES_KERN_RUN == KernCtrl.state) {

#if   (1u == CFG_HOOK_PRE_CTX_SW)
        userPreCtxSw(
            KernCtrl.cthd,
            KernCtrl.pthd);
#endif
            PORT_CTX_SW_ISR();

#if   (1u == CFG_SCHED_POWER_SAVE)
        } else if (ES_KERN_SLEEP == KernCtrl.state) {
            schedWakeUpI();
#endif
        }
    }
}


/*--  Virtual Timer management  ----------------------------------------------*/

void esKTmrInitI(
    esVTmr_T *          vTmr,
    esVTmrTick            tick,
    void (* fn)(void *),
    void *              arg) {

    ES_DEBUG_API_REQUIRE(ES_DEBUG_USAGE, ES_KERN_INACTIVE > KernCtrl.state);
    ES_DEBUG_API_REQUIRE(ES_DEBUG_POINTER, NULL != vTmr);
    ES_DEBUG_API_REQUIRE(ES_DEBUG_OBJECT, DEF_VTMR_CONTRACT_SIGNATURE != vTmr->signature);
    ES_DEBUG_API_REQUIRE(ES_DEBUG_RANGE, 1u < tick);
    ES_DEBUG_API_REQUIRE(ES_DEBUG_POINTER, NULL != fn);

    vTmr->rtick     = tick;
    vTmr->fn        = fn;
    vTmr->arg       = arg;
    vTmr->tmrL.q    = &VTmrPend;
    PQLIST_ENTRY_ADD_AFTER(tmrL, &VTmrPend, vTmr);
    ++SysTmr.vTmrPend;

#if   (1u == CFG_SYSTMR_ADAPTIVE_MODE)
    if (0u != SysTmr.ptick) {                                                   /* If system is sleeping we need to wake up VTmr thread.    */
        esThdPostI(
            &KVTmr);
    }
#endif
    ES_DEBUG_API_OBLIGATION(vTmr->signature = DEF_VTMR_CONTRACT_SIGNATURE);
}

void esVTmrInit(
    esVTmr_T *          vTmr,
    esVTmrTick            tick,
    void (* fn)(void *),
    void *              arg) {

    esIntrCtx           intCtx;

    ES_CRITICAL_LOCK_ENTER(&intCtx);
    esKTmrInitI(
        vTmr,
        tick,
        fn,
        arg);
    ES_CRITICAL_LOCK_EXIT(intCtx);
}

void esVTmrTermI(
    esVTmr_T *          vTmr) {

    ES_DEBUG_API_REQUIRE(ES_DEBUG_USAGE, ES_KERN_INACTIVE > KernCtrl.state);
    ES_DEBUG_API_REQUIRE(ES_DEBUG_POINTER, NULL != vTmr);
    ES_DEBUG_API_REQUIRE(ES_DEBUG_OBJECT, DEF_VTMR_CONTRACT_SIGNATURE == vTmr->signature);

    ES_DEBUG_API_OBLIGATION(vTmr->signature = ~DEF_VTMR_CONTRACT_SIGNATURE);

    if (&VTmrPend == vTmr->tmrL.q) {                                            /* A pending timer is being deleted.                        */
        DLIST_ENTRY_RM(tmrL, vTmr);
        --SysTmr.vTmrPend;
    } else {                                                                    /* An armed timer is being deleted.                         */

#if   (0u == CFG_SYSTMR_ADAPTIVE_MODE)
        esVTmr_T *      nextVTmr;

        --SysTmr.vTmrArmed;
        nextVTmr = PQLIST_ENTRY_NEXT(tmrL, vTmr);

        if (&VTmrArmed != nextVTmr) {
            nextVTmr->rtick += vTmr->rtick;
        }
        DLIST_ENTRY_RM(tmrL, vTmr);
#elif (1u == CFG_SYSTMR_ADAPTIVE_MODE)
        --SysTmr.vTmrArmed;

        if ((0u != SysTmr.ptick) &&
            (PQLIST_ENTRY_NEXT(tmrL_, &VTmrArmed) == vTmr)) {                    /* System timer was sleeping and vTmr is the current timer. */
            DLIST_ENTRY_RM(tmrL_, vTmr);

            if (0u != SysTmr.vTmrArmed) {                                       /* The last timer is not being deleted: remaining time is   */
                esVTmr_T * nextVTmr;                                            /* calculated to add to next timer in list.                 */

                vTmr->rtick -= (ES_SYSTIMER_GET_RVAL() - ES_SYSTIMER_GET_CVAL()) / ES_SYSTIMER_ONE_TICK;
                nextVTmr = PQLIST_ENTRY_NEXT(tmrL_, &VTmrArmed);
                nextVTmr->rtick += vTmr->rtick;
            }
            sysTmrDeactivateI();
        } else {
            esVTmr_T * nextVTmr;

            nextVTmr = PQLIST_ENTRY_NEXT(tmrL_, vTmr);

            if (&VTmrArmed != nextVTmr) {
                nextVTmr->rtick += vTmr->rtick;
            }
            DLIST_ENTRY_RM(tmrL_, vTmr);
        }
#endif
    }
}

void esVTmrTerm(
    esVTmr_T *          vTmr) {

    esIntrCtx           intrCtx;

    ES_CRITICAL_LOCK_ENTER(&intrCtx);
    esVTmrTermI(
        vTmr);
    ES_CRITICAL_LOCK_EXIT(intrCtx);
}

void esVTmrDelay(
    esVTmrTick            tick) {

    esVTmr_T            vTmr;

    esVTmrInit(
        &vTmr,
        tick,
        (void (*)(void *))thdPost,
        (void *)esThdGetId());
    thdWait();
}

/*--  Kernel time management  ------------------------------------------------*/

esVTmrTick esSysTmrTickGet(
    void) {

    esVTmrTick            tick;

    ES_DEBUG_API_REQUIRE(ES_DEBUG_USAGE, ES_KERN_INACTIVE > KernCtrl.state);

#if   (0u == CFG_SYSTMR_ADAPTIVE_MODE)
# if   (PORT_DATA_SIZE_VAL >= SYSTMR_TICK_TYPE_SIZE)
    tick = SysTmr.ctick;

    return (tick);
# else
    {
        esIntrCtx       intCtx;

        ES_CRITICAL_LOCK_ENTER(&intCtx);
        tick = SysTmr.ctick;
        ES_CRITICAL_LOCK_EXIT(intCtx);

        return (tick);
    }
# endif
#elif (1u == CFG_SYSTMR_ADAPTIVE_MODE)
    if (0u == SysTmr.ptick) {                                                   /* Normal operation.                                        */
        tick = SysTmr.ctick;
    } else {                                                                    /* Preempted wake up, system timer was enabled during sleep.*/

        if (0u != SysTmr.vTmrArmed) {                                           /* System timer was enabled during sleep.                   */
            portSysTmrReg_T tmrVal;

            tmrVal = (ES_SYSTIMER_GET_RVAL() - ES_SYSTIMER_GET_CVAL());
            tick = tmrVal / ES_SYSTIMER_ONE_TICK;
        } else {                                                                /* System timer was disabled during sleep.                  */
# if   (0u == CFG_SYSTMR_TICK_TYPE)
            tick = UINT8_MAX;
# elif (1u == CFG_SYSTMR_TICK_TYPE)
            tick = UINT16_MAX;
# elif (2u == CFG_SYSTMR_TICK_TYPE)
            tick = UINT32_MAX;
# endif
        }
    }

    return (tick);
#endif
}

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//** @} *//*********************************************
 * END of kernel.c
 ******************************************************************************/
