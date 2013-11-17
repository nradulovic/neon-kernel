/*
 * This file is part of eSolid - RT Kernel
 *
 * Copyright (C) 2011, 2012, 2013 - Nenad Radulovic
 *
 * eSolid - RT Kernel is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option) any
 * later version.
 *
 * eSolid - RT Kernel is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * eSolid - RT Kernel; if not, write to the Free Software Foundation, Inc., 51
 * Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * web site:    http://github.com/nradulovic
 * e-mail  :    nenad.b.radulovic@gmail.com
 *//***********************************************************************//**
 * @file
 * @author      Nenad Radulovic
 * @brief       Implementation of port independent code
 * @addtogroup  kern_impl
 *********************************************************************//** @{ */

/*=========================================================  INCLUDE FILES  ==*/

#include "kernel.h"

/*=========================================================  LOCAL MACRO's  ==*/

/**@brief       Priority Bit Map log base 2: `log2(PORT_DEF_DATA_WIDTH)`
 */
#define DEF_PBM_DATA_WIDTH_LOG2                                                 \
    (PORT_DEF_DATA_WIDTH <   2u ? 0u :                                          \
     (PORT_DEF_DATA_WIDTH <   4u ? 1u :                                         \
      (PORT_DEF_DATA_WIDTH <   8u ? 2u :                                        \
       (PORT_DEF_DATA_WIDTH <  16u ? 3u :                                       \
        (PORT_DEF_DATA_WIDTH <  32u ? 4u :                                      \
         (PORT_DEF_DATA_WIDTH <  64u ? 5u :                                     \
          (PORT_DEF_DATA_WIDTH < 128u ? 6u : 7u)))))))

/**@brief       Kernel state variable bit position which defines if the kernel
 *              is in interrupt servicing state.
 */
#define SCHED_STATE_INTSRV_MSK          (0x01u << 0)

/**@brief       Kernel state variable bit position which defines if the kernel
 *              is locked or not.
 */
#define SCHED_STATE_LOCK_MSK            (0x01u << 1)

/**@brief       Thread structure signature.
 * @details     The signature is used to confirm that a structure passed to a
 *              kernel function is indeed a esThd_T thread structure.
 */
#define THD_CONTRACT_SIGNATURE          ((portReg_T)0xfeedbeeful)

/**@brief       Thread Queue structure signature.
 * @details     The signature is used to confirm that a structure passed to a
 *              kernel function is indeed a esThdQ_T thread queue structure.
 */
#define THDQ_CONTRACT_SIGNATURE         ((portReg_T)0xfeedbef0ul)

/**@brief       Timer structure signature.
 * @details     The signature is used to confirm that a structure passed to a
 *              timer function is indeed a esVTmr_T timer structure.
 */
#define VTMR_CONTRACT_SIGNATURE         ((portReg_T)0xfeedbef1ul)

/**@brief       DList macro: is the thread the first one in the list
 */
#define DLIST_IS_ENTRY_FIRST(list, entry)                                       \
    ((entry) == (entry)->list.next)

/**@brief       DList macro: is the thread the last one in the list
 */
#define DLIST_IS_ENTRY_LAST(list, entry)                                        \
    DLIST_IS_ENTRY_FIRST(list, entry)

/**@brief       DList macro: is the thread single in the list
 */
#define DLIST_IS_ENTRY_SINGLE(list, entry)                                      \
    DLIST_IS_ENTRY_FIRST(list, entry)

/**@brief       DList macro: get the next entry
 */
#define DLIST_ENTRY_NEXT(list, entry)                                           \
    (entry)->list.next

/**@brief       DList macro: initialize entry
 */
#define DLIST_ENTRY_INIT(list, entry)                                           \
    do {                                                                        \
        (entry)->list.next = (entry);                                           \
        (entry)->list.prev = (entry);                                           \
    } while (0u)

/**@brief       DList macro: add new @c entry after @c current entry
 */
#define DLIST_ENTRY_ADD_AFTER(list, current, entry)                             \
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
    esTick_T            ctick;                                                  /**< @brief Current system timer tick value.                */
#if   (1u == CFG_SYSTMR_ADAPTIVE_MODE) || defined(__DOXYGEN__)
    esTick_T            ptick;                                                  /**< @brief Pending ticks during the timer sleep mode.      */
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

/**@brief       Set the bit corresponding to the prio argument
 * @param       pbm
 *              Pointer to the bit map structure
 * @param       prio
 *              Priority which will be marked as used
 */
static PORT_C_INLINE void pbmSet(
    struct pbm_ *       pbm,
    uint_fast8_t        prio);

/**@brief       Clear the bit corresponding to the prio argument
 * @param       pbm
 *              Pointer to the bit map structure
 * @param       prio
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
 *  @retval     TRUE - bit map is empty
 *  @retval     FALSE - there is at least one bit set
 */
static PORT_C_INLINE bool_T pbmIsEmpty(
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
    esThd_T *       thd);

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
    esTick_T            ticks);
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
    esThd_T *           thd);

/**@brief       Wait for a signal
 */
void thdWait(
    void);

/**@} *//*--------------------------------------------------------------------*/

/*=======================================================  LOCAL VARIABLES  ==*/

/**@brief       Module identification info
 */
DECL_MODULE_INFO("Kernel", "eSolid RT Kernel", "Nenad Radulovic");

/*------------------------------------------------------------------------*//**
 * @name        Scheduler
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Ready Thread queue
 */
static struct esThdQ RdyQueue;

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

/**@brief       List of virtual timers to armed expire
 */
static struct esVTmr VTmrArmed = {
   {    &VTmrArmed,
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
#if   (1u == CFG_DBG_API_VALIDATION)
   VTMR_CONTRACT_SIGNATURE
#endif
};

/**@brief       Virtual timers pending to be inserted into waiting list
 */
static struct esVTmr VTmrPend = {
   {    &VTmrPend,
        &VTmrPend,
        &VTmrPend
   },
   0u,
   NULL,
   NULL,
#if   (1u == CFG_DBG_API_VALIDATION)
   VTMR_CONTRACT_SIGNATURE
#endif
};

/**@brief       Virtual timer thread ID
 */
static struct esThd KVTmr;

/**@} *//*----------------------------------------------------------------*//**
 * @name        Idle kernel thread
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Idle thread ID
 */
static struct esThd KIdle;

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

    uint8_t             group;

#if   (1u != KERN_DEF_PBM_GRP_INDX_)
    pbm->bitGrp = 0u;
#endif

    for (group = 0u; group < KERN_DEF_PBM_GRP_INDX_; group++) {
        pbm->bit[group] = 0u;
    }
}

static PORT_C_INLINE void pbmSet(
    struct pbm_ *       pbm,
    uint_fast8_t        prio) {

#if   (1u != KERN_DEF_PBM_GRP_INDX_)
    uint_fast8_t        grpIndx;
    uint_fast8_t        bitIndx;

    bitIndx = prio & (~((uint_fast8_t)0u) >> (sizeof(prio) * 8u - DEF_PBM_DATA_WIDTH_LOG2));
    grpIndx = prio >> DEF_PBM_DATA_WIDTH_LOG2;
    pbm->bitGrp |= PORT_BIT_PWR2(grpIndx);
    pbm->bit[grpIndx] |= PORT_BIT_PWR2(bitIndx);
#else
    pbm->bit[0] |= PORT_BIT_PWR2(prio);
#endif
}

static PORT_C_INLINE void pbmClear(
    struct pbm_ *       pbm,
    uint_fast8_t        prio) {

#if   (1u != KERN_DEF_PBM_GRP_INDX_)
    uint_fast8_t        grpIndx;
    uint_fast8_t        bitIndx;

    bitIndx = prio & (~((uint_fast8_t)0u) >> (sizeof(prio) * 8u - DEF_PBM_DATA_WIDTH_LOG2));
    grpIndx = prio >> DEF_PBM_DATA_WIDTH_LOG2;
    pbm->bit[grpIndx] &= ~PORT_BIT_PWR2(bitIndx);

    if (0u == pbm->bit[grpIndx]) {                                              /* Is this the last one bit cleared in this group?          */
        pbm->bitGrp &= ~PORT_BIT_PWR2(grpIndx);                                 /* Yes: then clear bit group indicator, too.                */
    }
#else
    pbm->bit[0] &= ~PORT_BIT_PWR2(prio);
#endif
}

static PORT_C_INLINE uint_fast8_t pbmGetHighest(
    const struct pbm_ * pbm) {

#if   (1u != KERN_DEF_PBM_GRP_INDX_)
    uint_fast8_t        grpIndx;
    uint_fast8_t        bitIndx;

    grpIndx = PORT_BIT_FIND_LAST_SET(pbm->bitGrp);
    bitIndx = PORT_BIT_FIND_LAST_SET(pbm->bit[grpIndx]);

    return ((grpIndx << DEF_PBM_DATA_WIDTH_LOG2) | bitIndx);
#else
    uint_fast8_t        bitIndx;

    bitIndx = PORT_BIT_FIND_LAST_SET(pbm->bit[0]);

    return (bitIndx);
#endif
}

static PORT_C_INLINE bool_T pbmIsEmpty(
    const struct pbm_ * pbm) {

#if   (1u != KERN_DEF_PBM_GRP_INDX_)
    bool_T              ret;

    if (0u == pbm->bitGrp) {
        ret = TRUE;
    } else {
        ret = FALSE;
    }

    return (ret);
#else
    bool_T              ret;

    if (0u == pbm->bit[0]) {
        ret = TRUE;
    } else {
        ret = FALSE;
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

    portReg_T           intCtx;
    esThd_T *           nthd;

    ES_CRITICAL_ENTER(&intCtx);
    nthd = esThdQFetchI(                                                        /* Get the highest priority thread                          */
        &RdyQueue);
    ((struct kernCtrl_ *)&KernCtrl)->cthd  = nthd;
    ((struct kernCtrl_ *)&KernCtrl)->pthd  = nthd;
    ((struct kernCtrl_ *)&KernCtrl)->state = ES_KERN_RUN;
    ES_CRITICAL_EXIT(intCtx);
}

#if   (1u == CFG_SCHED_POWER_SAVE) || defined(__DOXYGEN__)
static PORT_C_INLINE void schedSleep(
    void) {

    portReg_T         intCtx;

    ES_CRITICAL_ENTER(&intCtx);
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
    PORT_CRITICAL_EXIT_SLEEP_ENTER();                                           /* Enter sleep state and wait for an interrupt.             */
# else
    ES_CRITICAL_EXIT(intCtx);
# endif
    ES_CRITICAL_ENTER(&intCtx);
    } while (KernCtrl.cthd == KernCtrl.pthd);
# if (1u == CFG_HOOK_POST_IDLE)
    userPostIdle();
# endif
    esKernLockExitI();
    ES_CRITICAL_EXIT(intCtx);
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
    esThd_T *           thd) {

    if (NULL == KernCtrl.pthd) {
        ((struct kernCtrl_ *)&KernCtrl)->pthd = thd;
    }
}

static PORT_C_INLINE void schedQmNextI(
    void) {

    esThd_T * nthd;
    esThd_T * cthd;

    cthd = KernCtrl.cthd;
    nthd = esThdQFetchRotateI(                                                  /* Fetch the next thread and rotate this priority group     */
        &RdyQueue,
        cthd->prio);

    if (cthd == KernCtrl.pthd) {                                                /* If there is no any other thread pending for switching    */
        ((struct kernCtrl_ *)&KernCtrl)->pthd = nthd;                           /* Make the new thread pending                              */
    }
}

static PORT_C_INLINE void schedQmI(
    void) {

    if (ES_KERN_LOCK > KernCtrl.state) {                                        /* Round-Robin is not enabled in kernel LOCK state          */
        esThd_T * cthd;

        cthd = KernCtrl.cthd;                                                   /* Get the current thread                                   */

        if (!DLIST_IS_ENTRY_SINGLE(thdL, cthd)) {
            cthd->qCnt--;                                                       /* Decrement current thread time quantum                    */

            if (0u == cthd->qCnt) {
                cthd->qCnt = cthd->qRld;                                        /* Reload thread time quantum                               */
                schedQmNextI();
            }
        }
    }
}


/*--  System timer  ----------------------------------------------------------*/

static PORT_C_INLINE void sysTmrInit(
    void) {

    PORT_SYSTMR_INIT(PORT_DEF_SYSTMR_ONE_TICK);
    PORT_SYSTMR_ENABLE();
    PORT_SYSTMR_ISR_ENABLE();
}

#if   (1u == CFG_SYSTMR_ADAPTIVE_MODE) || defined(__DOXYGEN__)
static PORT_C_INLINE void sysTmrActivate(
    void) {

    if (0u == SysTmr.ptick) {                                                   /* Normal wake up.                                          */

        if (0u != SysTmr.vTmrArmed) {                                           /* System timer was enabled during sleep.                   */
            portSysTmrReg_T tmrVal;

            tmrVal = PORT_SYSTMR_GET_RVAL() - PORT_SYSTMR_GET_CVAL();
            tmrVal = PORT_DEF_SYSTMR_ONE_TICK - tmrVal;
            PORT_SYSTMR_RLD(tmrVal);
        } else {                                                                /* System timer was disabled during sleep.                  */
            PORT_SYSTMR_RLD(PORT_DEF_SYSTMR_ONE_TICK);                          /* Reload macro will also re-enable the timer               */
            PORT_SYSTMR_ISR_ENABLE();
        }
    } else {                                                                    /* Preempted wake up, system timer was enabled during sleep.*/
        esVTmr_T *  vTmr;
        esTick_T    ticks;
        portSysTmrReg_T tmrVal;

        tmrVal = PORT_SYSTMR_GET_RVAL() - PORT_SYSTMR_GET_CVAL();
        ticks = tmrVal / PORT_DEF_SYSTMR_ONE_TICK;
        tmrVal -= (PORT_DEF_SYSTMR_ONE_TICK * ticks);
        tmrVal = PORT_DEF_SYSTMR_ONE_TICK - tmrVal;

        if (PORT_DEF_SYSTMR_WAKEUP_TH_VAL > tmrVal) {
            PORT_SYSTMR_RLD(tmrVal);
        } else {
            PORT_SYSTMR_RLD(tmrVal + PORT_DEF_SYSTMR_ONE_TICK);
        }
        vTmr = DLIST_ENTRY_NEXT(tmrL_, &VTmrArmed);
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

        vTmr = DLIST_ENTRY_NEXT(tmrL_, &VTmrArmed);
        vTmrSleep(
            vTmr->rtick);
    } else {                                                                    /* No virtual timer is armed: set system timer to disabled. */
        PORT_SYSTMR_ISR_DISABLE();
        PORT_SYSTMR_DISABLE();
    }
}
#endif


/*--  Virtual Timer and Virtual Timer kernel thread --------------------------*/

#if   (1u == CFG_SYSTMR_ADAPTIVE_MODE) || defined(__DOXYGEN__)
static PORT_C_INLINE void vTmrSleep(
    esTick_T            ticks) {

    portSysTmrReg_T sysTmrVal;

    if (PORT_DEF_SYSTMR_MAX_TICKS < ticks) {                                    /* Limit the number of ticks according to hardware maximum  */
        ticks = PORT_DEF_SYSTMR_MAX_TICKS;                                      /* specification.                                           */
    }
    SysTmr.ptick = ticks;

    if (0u != ticks) {
        sysTmrVal = PORT_DEF_SYSTMR_ONE_TICK * (ticks - 1u);
        sysTmrVal += PORT_SYSTMR_GET_CVAL() % PORT_DEF_SYSTMR_ONE_TICK;         /* Add the remaining time of current tick period.           */

        if (PORT_SYSTMR_GET_RVAL() != sysTmrVal) {
            PORT_SYSTMR_RLD(sysTmrVal);
        }
    }
}
#endif

static PORT_C_INLINE void vTmrEvaluateI(
    void) {
    ++SysTmr.ctick;

    if (0u != SysTmr.vTmrArmed) {                                               /* There is an armed timer waiting.                         */
        esVTmr_T * vTmr;

        vTmr = DLIST_ENTRY_NEXT(tmrL, &VTmrArmed);

#if   (0u == CFG_SYSTMR_ADAPTIVE_MODE)
        --vTmr->rtick;
#elif (1u == CFG_SYSTMR_ADAPTIVE_MODE)
        if (0u == SysTmr.ptick) {                                               /* Normal system tick.                                      */

            if (PORT_DEF_SYSTMR_ONE_TICK != PORT_SYSTMR_GET_RVAL()) {           /* If system timer is in adaptive mode switch it to fixed   */
                PORT_SYSTMR_RLD(PORT_DEF_SYSTMR_ONE_TICK);                      /* mode.                                                    */
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
    esTick_T            tick;

    vTmr->tmrL.q = &VTmrArmed;
    tmp = DLIST_ENTRY_NEXT(tmrL, &VTmrArmed);
    tick = vTmr->rtick;

    while (tmp->rtick <= tick) {
        tick -= tmp->rtick;
        tmp = DLIST_ENTRY_NEXT(tmrL, tmp);
    }
    vTmr->rtick = tick;
    DLIST_ENTRY_ADD_AFTER(tmrL, tmp, vTmr);

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
        tmr = DLIST_ENTRY_NEXT(tmrL_, &VTmrPend);
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

    portReg_T         lockCtx;

    ES_CRITICAL_ENTER(&lockCtx);
    esKernLockEnterI();

    while (0u != SysTmr.vTmrPend) {
        esVTmr_T * tmr;

        --SysTmr.vTmrPend;
        ++SysTmr.vTmrArmed;
        tmr = DLIST_ENTRY_NEXT(tmrL, &VTmrPend);
        DLIST_ENTRY_RM(tmrL, tmr);
        ES_CRITICAL_EXIT(lockCtx);
        --tmr->rtick;                                                           /* Timer thread requires one tick less than original value. */
        vTmrAddArmedS(
            tmr);
        ES_CRITICAL_ENTER(&lockCtx);
    }
    esKernLockExitI();
    ES_CRITICAL_EXIT(lockCtx);
}

/* 1)       Kernel Virtual Timer thread must have the highest priority available.
 */
static void kVTmrInit(
    void) {

    static portStck_T kVTmrStck[ES_STCK_SIZE(PORT_DEF_KVTMR_STCK_SIZE)];        /* Virtual timer kernel thread stack.                       */

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

    while (TRUE) {

        thdWait();

        if (0u != SysTmr.vTmrArmed) {                                           /* There is at least one armed timer.                       */
            esVTmr_T * tmr;

            tmr = DLIST_ENTRY_NEXT(tmrL, &VTmrArmed);

            while (0u == tmr->rtick) {
                esVTmr_T * tmpTmr;

                --SysTmr.vTmrArmed;
                DLIST_ENTRY_RM(tmrL, tmr);
                tmpTmr = tmr;
                ES_DBG_API_OBLIGATION(tmr->signature = ~VTMR_CONTRACT_SIGNATURE);
                tmr = DLIST_ENTRY_NEXT(tmrL, &VTmrArmed);
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

    static portStck_T kIdleStck[ES_STCK_SIZE(PORT_DEF_KIDLE_STCK_SIZE)];        /* Idle kernel thread stack.                                */

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

    while (TRUE) {
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
    esThd_T *           thd) {

    portReg_T           intCtx;

    ES_CRITICAL_ENTER(&intCtx);
    if (NULL == thd->thdL.q) {
        esSchedRdyAddI(
            thd);
        esSchedYieldI();
    }
    ES_CRITICAL_EXIT(intCtx);
}

void thdWait(
    void) {

    portReg_T           intCtx;

    ES_CRITICAL_ENTER(&intCtx);
    esSchedRdyRmI(
        esThdGetId());
    esSchedYieldI();
    ES_CRITICAL_EXIT(intCtx);
}

/*===================================  GLOBAL PRIVATE FUNCTION DEFINITIONS  ==*/
/*====================================  GLOBAL PUBLIC FUNCTION DEFINITIONS  ==*/


/*--  General kernel functions  ----------------------------------------------*/

void esKernInit(
    void) {

    ES_DBG_API_REQUIRE(ES_DBG_USAGE_FAILURE, ES_KERN_INACTIVE == KernCtrl.state);

#if   (1u == CFG_HOOK_PRE_KERN_INIT)
    userPreKernInit();
#endif
    PORT_INT_DISABLE();
    PORT_CPU_INIT_EARLY();
    sysTmrInit();
    schedInit();
    kIdleInit();
    kVTmrInit();
    PORT_CPU_INIT();
#if   (1u == CFG_HOOK_POST_KERN_INIT)
    userPostKernInit();
#endif
}

/* 1)       Since this function will never return it is marked with `noreturn`
 *          attribute to allow for compiler optimizations.
 */
PORT_C_NORETURN void esKernStart(
    void) {

    ES_DBG_API_REQUIRE(ES_DBG_USAGE_FAILURE, ES_KERN_INIT == KernCtrl.state);

#if   (1u == CFG_HOOK_PRE_KERN_START)
    userPreKernStart();
#endif
    PORT_CPU_INIT_LATE();
    schedStart();                                                               /* Initialize scheduler data structures for multi-threading */
    PORT_CTX_SW_START();                                                        /* Start the first thread                                   */

    while (TRUE);                                                               /* Prevent compiler `function does return` warnings.        */
}

void esKernSysTmr(
    void) {

    portReg_T         lockCtx;

    ES_DBG_API_REQUIRE(ES_DBG_USAGE_FAILURE, KernCtrl.cthd != &KVTmr);

#if   (1u == CFG_HOOK_PRE_SYSTMR_EVENT)
    userPreSysTmr();
#endif
    ES_CRITICAL_ENTER(
        &lockCtx);
    vTmrEvaluateI();
    schedQmI();
    ES_CRITICAL_EXIT(
        lockCtx);
}

void esKernIsrEnterI(
    void) {

    ES_DBG_API_REQUIRE(ES_DBG_USAGE_FAILURE, ES_KERN_INIT > KernCtrl.state);

    ((struct kernCtrl_ *)&KernCtrl)->state |= SCHED_STATE_INTSRV_MSK;
}

void esKernIsrExitI(
    void) {

    ES_DBG_API_REQUIRE(ES_DBG_USAGE_FAILURE, ES_KERN_INIT > KernCtrl.state);

    if (TRUE == PORT_ISR_IS_LAST()) {
        ((struct kernCtrl_ *)&KernCtrl)->state &= ~SCHED_STATE_INTSRV_MSK;
        esSchedYieldIsrI();
    }
}


/*--  Critical code section locking management  ------------------------------*/

void esKernLockIntEnter(
    esLockCtx_T *       lockCtx) {

    PORT_INT_PRIO_REPLACE(lockCtx, PORT_DEF_MAX_ISR_PRIO);
    esKernLockEnterI();
}

void esKernLockIntExit(
    esLockCtx_T         lockCtx) {

    esKernLockExitI();
    PORT_INT_PRIO_SET(lockCtx);
}

void esKernLockEnterI(
    void) {

    ES_DBG_API_REQUIRE(ES_DBG_USAGE_FAILURE, ES_KERN_INIT > KernCtrl.state);

    ((struct kernCtrl_ *)&KernCtrl)->state |= SCHED_STATE_LOCK_MSK;
    ++KernLockCnt;
}

void esKernLockExitI(
    void) {

    ES_DBG_API_REQUIRE(ES_DBG_USAGE_FAILURE, ES_KERN_LOCK == KernCtrl.state);
    ES_DBG_API_REQUIRE(ES_DBG_USAGE_FAILURE, 0u != KernLockCnt);

    --KernLockCnt;

    if (0u == KernLockCnt) {
        ((struct kernCtrl_ *)&KernCtrl)->state &= ~SCHED_STATE_LOCK_MSK;
        esSchedYieldI();
    }
}

void esKernLockEnter(
    void) {

    portReg_T           intCtx;

    ES_CRITICAL_ENTER(&intCtx);
    esKernLockEnterI();
    ES_CRITICAL_EXIT(intCtx);
}

void esKernLockExit(
    void) {

    portReg_T           intCtx;

    ES_CRITICAL_ENTER(&intCtx);
    esKernLockExitI();
    ES_CRITICAL_EXIT(intCtx);
}


/*--  Thread management ------------------------------------------------------*/

void esThdInit(
    esThd_T *           thd,
    void (* fn)(void *),
    void *              arg,
    portStck_T *        stck,
    size_t              stckSize,
    uint8_t             prio) {

    portReg_T           intCtx;

    ES_DBG_API_REQUIRE(ES_DBG_USAGE_FAILURE, ES_KERN_INACTIVE > KernCtrl.state);
    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL != thd);
    ES_DBG_API_REQUIRE(ES_DBG_OBJECT_NOT_VALID, THD_CONTRACT_SIGNATURE != thd->signature);
    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL != fn);
    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL != stck);
    ES_DBG_API_REQUIRE(ES_DBG_OUT_OF_RANGE, PORT_DEF_STCK_MINSIZE <= (stckSize * sizeof(portReg_T)));
    ES_DBG_API_REQUIRE(ES_DBG_OUT_OF_RANGE, CFG_SCHED_PRIO_LVL > prio);
    ES_DBG_API_REQUIRE(ES_DBG_OUT_OF_RANGE, ((&KVTmr != thd) && ((CFG_SCHED_PRIO_LVL - 1u) > prio)) || (&KVTmr == thd));
    ES_DBG_API_REQUIRE(ES_DBG_OUT_OF_RANGE, ((&KIdle != thd) && (0u < prio)) || (&KIdle == thd));

    thd->stck   = PORT_CTX_INIT(stck, stckSize, fn, arg);                       /* Make a fake thread stack.                                */
    thd->thdL.q = NULL;                                                         /* This thread is not in any thread queue.                  */
    DLIST_ENTRY_INIT(thdL, thd);
    thd->prio   = prio;                                                         /* Set the priority.                                        */
    thd->iprio  = prio;                                                         /* This is constant priority, it never changes.             */
    thd->qCnt   = CFG_SCHED_TIME_QUANTUM;
    thd->qRld   = CFG_SCHED_TIME_QUANTUM;

    ES_DBG_API_OBLIGATION(thd->signature = THD_CONTRACT_SIGNATURE);             /* Make thread structure valid.                             */

    ES_CRITICAL_ENTER(&intCtx);
    schedRdyAddInitI(
        thd);                                                                   /* Initialize thread before adding it to Ready Thread queue.*/
    esSchedRdyAddI(
        thd);                                                                   /* Add the thread to Ready Thread queue.                    */
    esSchedYieldI();                                                            /* Invoke the scheduler.                                    */
    ES_CRITICAL_EXIT(intCtx);

#if   (1u == CFG_HOOK_POST_THD_INIT)
    userPostThdInit();
#endif
}

void esThdTerm(
    esThd_T *           thd) {

    portReg_T           intCtx;

    ES_DBG_API_REQUIRE(ES_DBG_USAGE_FAILURE, ES_KERN_INACTIVE > KernCtrl.state);
    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL != thd);
    ES_DBG_API_REQUIRE(ES_DBG_OBJECT_NOT_VALID, THD_CONTRACT_SIGNATURE == thd->signature);
    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, (NULL == thd->thdL.q) || (&RdyQueue == thd->thdL.q));

#if   (1u == CFG_HOOK_PRE_THD_TERM)
    userPreThdTerm();
#endif
    ES_CRITICAL_ENTER(&intCtx);

    if (&RdyQueue == thd->thdL.q) {
        esSchedRdyRmI(
            thd);
    } else if (NULL != thd->thdL.q) {
        esThdQRmI(
            thd->thdL.q,
            thd);
    }

    ES_DBG_API_OBLIGATION(thd->signature = ~THD_CONTRACT_SIGNATURE);            /* Mark the thread ID structure as invalid.                 */

    esSchedYieldI();
    ES_CRITICAL_EXIT(intCtx);
}

void esThdSetPrioI(
    esThd_T *           thd,
    uint8_t             prio) {

    ES_DBG_API_REQUIRE(ES_DBG_USAGE_FAILURE, ES_KERN_INACTIVE > KernCtrl.state);
    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL != thd);
    ES_DBG_API_REQUIRE(ES_DBG_OBJECT_NOT_VALID, THD_CONTRACT_SIGNATURE == thd->signature);
    ES_DBG_API_REQUIRE(ES_DBG_OUT_OF_RANGE, CFG_SCHED_PRIO_LVL >= prio);

    if (NULL == thd->thdL.q) {                                                  /* Is thread inserted in any queue?                         */
        thd->prio = prio;                                                       /* Just change it's prio value.                             */
    } else {
        esThdQRmI(
            thd->thdL.q,
            thd);
        thd->prio = prio;
        esThdQAddI(
            thd->thdL.q,
            thd);

        if (&RdyQueue == thd->thdL.q) {                                         /* Is thread in ready thread queue?                         */

            if (thd->prio > KernCtrl.pthd->prio) {                              /* If new prio is higher than the current prio notify the   */
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
    esThdQ_T *          thdQ) {

    uint_fast8_t        group;

    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL != thdQ);
    ES_DBG_API_REQUIRE(ES_DBG_OBJECT_NOT_VALID, THDQ_CONTRACT_SIGNATURE != thdQ->signature);

    pbmInit(
        &thdQ->prioOcc);

    for (group = 0u; group < CFG_SCHED_PRIO_LVL; group++) {
        thdQ->grp[group].head = NULL;
        thdQ->grp[group].next  = NULL;
    }
    ES_DBG_API_OBLIGATION(thdQ->signature = THDQ_CONTRACT_SIGNATURE);
}

/* 1)       When API validation is not used then this function will become empty.
 */
void esThdQTerm(
    esThdQ_T *          thdQ) {

    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL != thdQ);
    ES_DBG_API_REQUIRE(ES_DBG_OBJECT_NOT_VALID, THDQ_CONTRACT_SIGNATURE == thdQ->signature);

    ES_DBG_API_OBLIGATION(thdQ->signature = ~THDQ_CONTRACT_SIGNATURE);

#if   (0u == CFG_DBG_API_VALIDATION)
    (void)thdQ;                                                                 /* Prevent compiler warning about unused argument.          */
#endif
}

void esThdQAddI(
    esThdQ_T *          thdQ,
    esThd_T *           thd) {

    struct thdLSent_ *  sentinel;

    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL != thdQ);
    ES_DBG_API_REQUIRE(ES_DBG_OBJECT_NOT_VALID, THDQ_CONTRACT_SIGNATURE == thdQ->signature);
    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL != thd);
    ES_DBG_API_REQUIRE(ES_DBG_OBJECT_NOT_VALID, THD_CONTRACT_SIGNATURE == thd->signature);
    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL == thd->thdL.q);

    sentinel = &(thdQ->grp[thd->prio]);                                         /* Get the sentinel from thread priority level.             */

    if (NULL == sentinel->head) {                                               /* Is thdL_ list empty?                                      */
        sentinel->head = thd;                                                   /* This thread becomes first in the list.                   */
        sentinel->next = thd;
        pbmSet(
            &thdQ->prioOcc,
            thd->prio);                                                         /* Mark the priority group as used.                         */
    } else {
        DLIST_ENTRY_ADD_AFTER(thdL, sentinel->head, thd);                       /* Thread is added at the next of the list.                 */
    }
    thd->thdL.q = thdQ;                                                         /* Set the pointer to the thread queue being used.          */
}

void esThdQRmI(
    esThdQ_T *          thdQ,
    esThd_T *           thd) {

    struct thdLSent_ *  sentinel;

    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL != thd);
    ES_DBG_API_REQUIRE(ES_DBG_OBJECT_NOT_VALID, THD_CONTRACT_SIGNATURE == thd->signature);
    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL != thdQ);
    ES_DBG_API_REQUIRE(ES_DBG_OBJECT_NOT_VALID, THDQ_CONTRACT_SIGNATURE == thdQ->signature);
    ES_DBG_API_REQUIRE(ES_DBG_USAGE_FAILURE, thdQ == thd->thdL.q);

    sentinel = &(thdQ->grp[thd->prio]);                                         /* Get the sentinel from thread priority level.             */

    if (DLIST_IS_ENTRY_LAST(thdL, thd)) {                                       /* Is this thread last one in the thdL_ list?                */
        sentinel->head = NULL;                                                  /* Make the list sentinel empty.                            */
        pbmClear(
            &thdQ->prioOcc,
            thd->prio);                                                         /* Remove the mark since this group is not used.            */
    } else {                                                                    /* This thread is not the last one in the thdL_ list.        */

        if (sentinel->head == thd) {                                            /* In case we are removing first thread in linked list then */
            sentinel->head = DLIST_ENTRY_NEXT(thdL, thd);                       /* advance the head to point to the next one in the list.   */
        }

        if (sentinel->next == thd) {                                            /* In case we are removing next thread in the linked list   */
            sentinel->next = DLIST_ENTRY_NEXT(thdL, thd);                       /* then move next to point to a next one in the list.       */
        }
        DLIST_ENTRY_RM(thdL, thd);
        DLIST_ENTRY_INIT(thdL, thd);
    }
    thd->thdL.q = NULL;
}

esThd_T * esThdQFetchI(
    const esThdQ_T *    thdQ) {

    struct thdLSent_ *  sentinel;
    uint_fast8_t        prio;

    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL != thdQ);
    ES_DBG_API_REQUIRE(ES_DBG_OBJECT_NOT_VALID, THDQ_CONTRACT_SIGNATURE == thdQ->signature);
    ES_DBG_API_REQUIRE(ES_DBG_USAGE_FAILURE, FALSE == pbmIsEmpty(&thdQ->prioOcc));

    prio = pbmGetHighest(
        &thdQ->prioOcc);                                                        /* Get the highest priority ready to run.                   */
    sentinel = (struct thdLSent_ *)&(thdQ->grp[prio]);                          /* Get the Group Head pointer for that priority.            */
                                                                                /* The type cast is needed to avoid compiler warnings.      */
    ES_DBG_API_ENSURE(ES_DBG_OBJECT_NOT_VALID, THD_CONTRACT_SIGNATURE == sentinel->next->signature);

    return (sentinel->next);
}

esThd_T * esThdQFetchRotateI(
    esThdQ_T *          thdQ,
    uint_fast8_t        prio) {

    struct thdLSent_ *  sentinel;

    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL != thdQ);
    ES_DBG_API_REQUIRE(ES_DBG_OBJECT_NOT_VALID, THDQ_CONTRACT_SIGNATURE == thdQ->signature);
    ES_DBG_API_REQUIRE(ES_DBG_OUT_OF_RANGE, CFG_SCHED_PRIO_LVL >= prio);
    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL != thdQ->grp[prio].next);

    sentinel = &(thdQ->grp[prio]);                                              /* Get the Group Head pointer from thread priority.         */
    sentinel->next = DLIST_ENTRY_NEXT(thdL, sentinel->next);

    ES_DBG_API_ENSURE(ES_DBG_OBJECT_NOT_VALID, THD_CONTRACT_SIGNATURE == sentinel->next->signature);

    return (sentinel->next);
}

bool_T esThdQIsEmpty(
    const esThdQ_T *    thdQ) {

    bool_T              ret;

    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL != thdQ);
    ES_DBG_API_REQUIRE(ES_DBG_OBJECT_NOT_VALID, THDQ_CONTRACT_SIGNATURE == thdQ->signature);

    ret = pbmIsEmpty(
        &thdQ->prioOcc);

    return (ret);
}


/*--  Scheduler notification and invocation  ---------------------------------*/

void esSchedRdyAddI(
    esThd_T *           thd) {

    ES_DBG_API_REQUIRE(ES_DBG_USAGE_FAILURE, ES_KERN_INACTIVE > KernCtrl.state);
    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL != thd);
    ES_DBG_API_REQUIRE(ES_DBG_OBJECT_NOT_VALID, THD_CONTRACT_SIGNATURE == thd->signature);
    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL == thd->thdL.q);

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
    esThd_T *           thd) {

    ES_DBG_API_REQUIRE(ES_DBG_USAGE_FAILURE, ES_KERN_INACTIVE > KernCtrl.state);
    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL != thd);
    ES_DBG_API_REQUIRE(ES_DBG_OBJECT_NOT_VALID, THD_CONTRACT_SIGNATURE == thd->signature);
    ES_DBG_API_REQUIRE(ES_DBG_USAGE_FAILURE, &RdyQueue == thd->thdL.q);

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

    ES_DBG_API_REQUIRE(ES_DBG_USAGE_FAILURE, ES_KERN_INACTIVE > KernCtrl.state);

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

    ES_DBG_API_REQUIRE(ES_DBG_USAGE_FAILURE, ES_KERN_INACTIVE > KernCtrl.state);

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

void esVTmrInitI(
    esVTmr_T *          vTmr,
    esTick_T            tick,
    void (* fn)(void *),
    void *              arg) {

    ES_DBG_API_REQUIRE(ES_DBG_USAGE_FAILURE, ES_KERN_INACTIVE > KernCtrl.state);
    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL != vTmr);
    ES_DBG_API_REQUIRE(ES_DBG_OBJECT_NOT_VALID, VTMR_CONTRACT_SIGNATURE != vTmr->signature);
    ES_DBG_API_REQUIRE(ES_DBG_OUT_OF_RANGE, 1u < tick);
    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL != fn);

    vTmr->rtick     = tick;
    vTmr->fn        = fn;
    vTmr->arg       = arg;
    vTmr->tmrL.q    = &VTmrPend;
    DLIST_ENTRY_ADD_AFTER(tmrL, &VTmrPend, vTmr);
    ++SysTmr.vTmrPend;

#if   (1u == CFG_SYSTMR_ADAPTIVE_MODE)
    if (0u != SysTmr.ptick) {                                                   /* If system is sleeping we need to wake up VTmr thread.    */
        esThdPostI(
            &KVTmr);
    }
#endif
    ES_DBG_API_OBLIGATION(vTmr->signature = VTMR_CONTRACT_SIGNATURE);
}

void esVTmrInit(
    esVTmr_T *          vTmr,
    esTick_T            tick,
    void (* fn)(void *),
    void *              arg) {

    portReg_T           intCtx;

    ES_CRITICAL_ENTER(&intCtx);
    esVTmrInitI(
        vTmr,
        tick,
        fn,
        arg);
    ES_CRITICAL_EXIT(intCtx);
}

void esVTmrTermI(
    esVTmr_T *          vTmr) {

    ES_DBG_API_REQUIRE(ES_DBG_USAGE_FAILURE, ES_KERN_INACTIVE > KernCtrl.state);
    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL != vTmr);
    ES_DBG_API_REQUIRE(ES_DBG_OBJECT_NOT_VALID, VTMR_CONTRACT_SIGNATURE == vTmr->signature);

    ES_DBG_API_OBLIGATION(vTmr->signature = ~VTMR_CONTRACT_SIGNATURE);

    if (&VTmrPend == vTmr->tmrL.q) {                                            /* A pending timer is being deleted.                        */
        DLIST_ENTRY_RM(tmrL, vTmr);
        --SysTmr.vTmrPend;
    } else {                                                                    /* An armed timer is being deleted.                         */

#if   (0u == CFG_SYSTMR_ADAPTIVE_MODE)
        esVTmr_T *      nextVTmr;

        --SysTmr.vTmrArmed;
        nextVTmr = DLIST_ENTRY_NEXT(tmrL, vTmr);

        if (&VTmrArmed != nextVTmr) {
            nextVTmr->rtick += vTmr->rtick;
        }
        DLIST_ENTRY_RM(tmrL, vTmr);
#elif (1u == CFG_SYSTMR_ADAPTIVE_MODE)
        --SysTmr.vTmrArmed;

        if ((0u != SysTmr.ptick) &&
            (DLIST_ENTRY_NEXT(tmrL_, &VTmrArmed) == vTmr)) {                    /* System timer was sleeping and vTmr is the current timer. */
            DLIST_ENTRY_RM(tmrL_, vTmr);

            if (0u != SysTmr.vTmrArmed) {                                       /* The last timer is not being deleted: remaining time is   */
                esVTmr_T * nextVTmr;                                            /* calculated to add to next timer in list.                 */

                vTmr->rtick -= (PORT_SYSTMR_GET_RVAL() - PORT_SYSTMR_GET_CVAL()) / PORT_DEF_SYSTMR_ONE_TICK;
                nextVTmr = DLIST_ENTRY_NEXT(tmrL_, &VTmrArmed);
                nextVTmr->rtick += vTmr->rtick;
            }
            sysTmrDeactivateI();
        } else {
            esVTmr_T * nextVTmr;

            nextVTmr = DLIST_ENTRY_NEXT(tmrL_, vTmr);

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

    portReg_T           intCtx;

    ES_CRITICAL_ENTER(&intCtx);
    esVTmrTermI(
        vTmr);
    ES_CRITICAL_EXIT(intCtx);
}

void esVTmrDelay(
    esTick_T            tick) {

    esVTmr_T            vTmr;

    esVTmrInit(
        &vTmr,
        tick,
        (void (*)(void *))thdPost,
        (void *)esThdGetId());
    thdWait();
}

/*--  Kernel time management  ------------------------------------------------*/

esTick_T esSysTmrTickGet(
    void) {

    esTick_T            tick;

    ES_DBG_API_REQUIRE(ES_DBG_USAGE_FAILURE, ES_KERN_INACTIVE > KernCtrl.state);

#if   (0u == CFG_SYSTMR_ADAPTIVE_MODE)
# if   (PORT_DATA_SIZE_VAL >= SYSTMR_TICK_TYPE_SIZE)
    tick = SysTmr.ctick;

    return (tick);
# else
    {
        portReg_T       intCtx;

        ES_CRITICAL_ENTER(&intCtx);
        tick = SysTmr.ctick;
        ES_CRITICAL_EXIT(intCtx);

        return (tick);
    }
# endif
#elif (1u == CFG_SYSTMR_ADAPTIVE_MODE)
    if (0u == SysTmr.ptick) {                                                   /* Normal operation.                                        */
        tick = SysTmr.ctick;
    } else {                                                                    /* Preempted wake up, system timer was enabled during sleep.*/

        if (0u != SysTmr.vTmrArmed) {                                           /* System timer was enabled during sleep.                   */
            portSysTmrReg_T tmrVal;

            tmrVal = (PORT_SYSTMR_GET_RVAL() - PORT_SYSTMR_GET_CVAL());
            tick = tmrVal / PORT_DEF_SYSTMR_ONE_TICK;
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
/** @endcond *//** @} *//******************************************************
 * END of kernel.c
 ******************************************************************************/
