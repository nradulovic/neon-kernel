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
 * @author      Nenad Radulovic
 * @brief       Implementation of port independent code
 * @addtogroup  kern_impl
 *********************************************************************//** @{ */

/*=========================================================  INCLUDE FILES  ==*/
#include "kernel.h"

/*=========================================================  LOCAL MACRO's  ==*/

/**@brief       Priority Bit Map log base 2: `log2(PORT_DATA_WIDTH_VAL)`
 */
#define PRIO_BM_DATA_WIDTH_LOG2                                                 \
    (PORT_DATA_WIDTH_VAL <   2 ? 0 :                                            \
     (PORT_DATA_WIDTH_VAL <   4 ? 1 :                                           \
      (PORT_DATA_WIDTH_VAL <   8 ? 2 :                                          \
       (PORT_DATA_WIDTH_VAL <  16 ? 3 :                                         \
        (PORT_DATA_WIDTH_VAL <  32 ? 4 :                                        \
         (PORT_DATA_WIDTH_VAL <  64 ? 5 :                                       \
          (PORT_DATA_WIDTH_VAL < 128 ? 6 : 7)))))))

/**@brief       Kernel state variable bit position which defines if kernel is in
 *              interrupt servicing state
 */
#define SCHED_STATE_INTSRV_MSK          (1U << 0)

/**@brief       Kernel state variable bit position which defines if the kernel
 *              is locked or not.
 */
#define SCHED_STATE_LOCK_MSK            (1U << 1)

/**@brief       Thread structure signature
 * @details     The signature is used to confirm that a structure passed to a
 *              kernel function is indeed a esThd_T thread structure.
 */
#define THD_CONTRACT_SIGNATURE          ((portReg_T)0xFEEDBEEFU)

/**@brief       Thread Queue structure signature
 * @details     The signature is used to confirm that a structure passed to a
 *              kernel function is indeed a esThdQ_T thread queue structure.
 */
#define THDQ_CONTRACT_SIGNATURE         ((portReg_T)0xFEEDBEEEU)

/**@brief       Timer structure signature
 * @details     The signature is used to confirm that a structure passed to a
 *              timer function is indeed a esVTmr_T timer structure.
 */
#define VTMR_CONTRACT_SIGNATURE         ((portReg_T)0xFEEDBEEFU)

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
    } while (0U)

/**@brief       DList macro: add new @c entry after @c current entry
 */
#define DLIST_ENTRY_ADD_AFTER(list, current, entry)                             \
    do {                                                                        \
        (entry)->list.next = (current);                                         \
        (entry)->list.prev = (entry)->list.next->list.prev;                     \
        (entry)->list.next->list.prev = (entry);                                \
        (entry)->list.prev->list.next = (entry);                                \
    } while (0U)

/**@brief       DList macro: remove the @c entry from a list
 */
#define DLIST_ENTRY_RM(list, entry)                                             \
    do {                                                                        \
        (entry)->list.next->list.prev = (entry)->list.prev;                     \
        (entry)->list.prev->list.next = (entry)->list.next;                     \
    } while (0U)

/*======================================================  LOCAL DATA TYPES  ==*/

/*------------------------------------------------------------------------*//**
 * @name        System Timer
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Main System Timer structure
 * @note        1) Member `ptick` exists only if ADAPTIVE mode is selected. When
 *              this mode is selected then kernel supports more aggresive power
 *              savings.
 */
struct sysTmr {
    uint_fast16_t       vTmrArmed;                                              /**< @brief The number of armed virtual timers in system.   */
    uint_fast16_t       vTmrPend;                                               /**< @brief The number of pending timers for arming.        */
#if (1U == CFG_SYSTMR_ADAPTIVE_MODE) || defined(__DOXYGEN__)
    esTick_T            ptick;                                                  /**< @brief Pending ticks during the timer sleep mode.      */
#endif
};

/**@brief       System Timer type
 */
typedef struct sysTmr sysTmr_T;

/**@} *//*----------------------------------------------------------------*//**
 * @name        Priority Bit Map
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Priority Bit Map type
 */
typedef struct prioBM prioBM_T;

/**@} *//*----------------------------------------------------------------*//**
 * @name        Threads Queue
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Thread list sentinel type
 */
typedef struct thdLSentinel thdLSentinel_T;

/**@} *//*--------------------------------------------------------------------*/
/*=============================================  LOCAL FUNCTION PROTOTYPES  ==*/

/*------------------------------------------------------------------------*//**
 * @name        Priority Bit Map
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Initialize bitmap
 * @param       bm
 *              Pointer to the bit map structure
 */
static PORT_C_INLINE void prioBMInit(
    prioBM_T *      bm);

/**@brief       Set the bit corresponding to the prio argument
 * @param       bm
 *              Pointer to the bit map structure
 * @param       prio
 *              Priority which will be marked as used
 */
static PORT_C_INLINE void prioBMSet(
    prioBM_T *      bm,
    uint_fast8_t    prio);

/**@brief       Clear the bit corresponding to the prio argument
 * @param       bm
 *              Pointer to the bit map structure
 * @param       prio
 *              Priority which will be marked as unused
 */
static PORT_C_INLINE void prioBMClear(
    prioBM_T *      bm,
    uint_fast8_t    prio);

/**@brief       Get the highest priority set
 * @param       bm
 *              Pointer to the bit map structure
 * @return      The number of the highest priority marked as used
 */
static PORT_C_INLINE uint_fast8_t prioBMGet(
    const prioBM_T *    bm);

/**@brief       Is bit map empty?
 * @param       bm
 *              Pointer to the bit map structure
 * @return      The status of the bit map
 *  @retval     TRUE - bit map is empty
 *  @retval     FALSE - there is at least one bit set
 */
static PORT_C_INLINE bool_T prioBMIsEmpty(
    const prioBM_T *    bm);


/**@} *//*----------------------------------------------------------------*//**
 * @name        Scheduler
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Initialize Ready Thread Queue structure @ref gRdyQueue and
 *              Kernel control structure @ref esKernCtrl.
 */
static PORT_C_INLINE void schedInit(
    void);

/**@brief       Set the scheduler data structures for multi-threading
 * @details     This function is called just before multi-threading will start.
 */
static PORT_C_INLINE void schedStart(
    void);

/**@brief       Set the scheduler to sleep
 */
#if (1U == CFG_SCHED_POWER_SAVE) || defined(__DOXYGEN__)
static PORT_C_INLINE void schedSleep(
    void);
#endif

/**@brief       Wake up the scheduler
 */
#if (1U == CFG_SCHED_POWER_SAVE) || defined(__DOXYGEN__)
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
 */
#if (1U == CFG_SYSTMR_ADAPTIVE_MODE) || defined(__DOXYGEN__)
static PORT_C_INLINE void sysTmrActivate(
    void);
#endif

/**@brief       Try to deactivate system timer
 */
#if (1U == CFG_SYSTMR_ADAPTIVE_MODE) || defined(__DOXYGEN__)
static PORT_C_INLINE void sysTmrDeactivateI(
    void);
#endif

/**@} *//*----------------------------------------------------------------*//**
 * @name        Virtual Timer kernel thread
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Set up system timer for different tick period during sleeping
 */
#if (1U == CFG_SYSTMR_ADAPTIVE_MODE) || defined(__DOXYGEN__)
static PORT_C_INLINE void vTmrSleep(
    esTick_T        ticks);
#endif

/**@brief       Evaluate armed virtual timers
 */
static PORT_C_INLINE void vTmrEvaluateI(
    void);

/**@brief       Add a virtual timer into sorted list
 * @param       tmr
 *              Virtual timer: pointer to virtual timer to add
 */
static void vTmrAddArmed(
    esVTmr_T *       vTmr);

#if (1U == CFG_SYSTMR_ADAPTIVE_MODE) || defined(__DOXYGEN__)
static PORT_C_INLINE void vTmrImportPendI(
    void);
#endif

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
    void *          arg);

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
    void *          arg);

/**@} *//*--------------------------------------------------------------------*/
/*=======================================================  LOCAL VARIABLES  ==*/

/*------------------------------------------------------------------------*//**
 * @name        Scheduler
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Ready Thread queue
 */
static esThdQ_T gRdyQueue;

/**@} *//*----------------------------------------------------------------*//**
 * @name        System timer kernel thread
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Main System Timer structure
 */
static sysTmr_T gSysTmr = {
    0U,
    0U,
#if (1U == CFG_SYSTMR_ADAPTIVE_MODE)
    0U
#endif
};

/**@brief       List of virtual timers to armed expire
 */
static esVTmr_T gVTmrArmed = {
   {    &gVTmrArmed,
        &gVTmrArmed,
        &gVTmrArmed
   },

#if (0U == CFG_SYSTMR_TICK_TYPE)
   UINT_FAST8_MAX,
#elif (1U == CFG_SYSTMR_TICK_TYPE)
   UINT_FAST16_MAX,
#elif (2U == CFG_SYSTMR_TICK_TYPE)
   UINT_FAST32_MAX,
#endif
   NULL,
   NULL,
#if (1U == CFG_DBG_API_VALIDATION)
   VTMR_CONTRACT_SIGNATURE
#endif
};

/**@brief       Virtual timers pending to be inserted into waiting list
 */
static esVTmr_T gVTmrPend = {
   {    &gVTmrPend,
        &gVTmrPend,
        &gVTmrPend
   },
   0U,
   NULL,
   NULL,
#if (1U == CFG_DBG_API_VALIDATION)
   VTMR_CONTRACT_SIGNATURE
#endif
};

/**@brief       Virtual timer thread ID
 */
static esThd_T gKVTmr;

/**@} *//*----------------------------------------------------------------*//**
 * @name        Idle kernel thread
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Idle thread ID
 */
static esThd_T gKIdle;

/**@} *//*--------------------------------------------------------------------*/

/**@brief       Kernel Lock Counter
 */
static uint_fast8_t gKernLockCnt;

/*======================================================  GLOBAL VARIABLES  ==*/

/**@brief       Kernel control initialization
 */
const volatile esKernCtrl_T gKernCtrl = {
    NULL,                                                                       /* No thread is currently executing                         */
    NULL,                                                                       /* No thread is pending                                     */
    ES_KERN_INACTIVE                                                            /* This is default kernel state before initialization       */
};

/*============================================  LOCAL FUNCTION DEFINITIONS  ==*/


/*--  Priority Bit Map functions  --------------------------------------------*/

static PORT_C_INLINE void prioBMInit(
    prioBM_T *      bm) {

    uint8_t         group;

#if (1U != PRIO_BM_GRP_INDX)
    bm->bitGrp = 0U;
#endif

    for (group = 0U; group < PRIO_BM_GRP_INDX; group++) {
        bm->bit[group] = 0U;
    }
}

static PORT_C_INLINE void prioBMSet(
    prioBM_T *      bm,
    uint_fast8_t    prio) {

#if (1U != PRIO_BM_GRP_INDX)
    uint_fast8_t    grpIndx;
    uint_fast8_t    bitIndx;

    bitIndx = prio & (~((uint_fast8_t)0U) >> (sizeof(prio) * 8U - PRIO_BM_DATA_WIDTH_LOG2));
    grpIndx = prio >> PRIO_BM_DATA_WIDTH_LOG2;
    bm->bitGrp |= PORT_PWR2(grpIndx);
    bm->bit[grpIndx] |= PORT_PWR2(bitIndx);
#else
    bm->bit[0] |= PORT_PWR2(prio);
#endif
}

static PORT_C_INLINE void prioBMClear(
    prioBM_T *      bm,
    uint_fast8_t    prio) {

#if (1U != PRIO_BM_GRP_INDX)
    uint_fast8_t    grpIndx;
    uint_fast8_t    bitIndx;

    bitIndx = prio & (~((uint_fast8_t)0U) >> (sizeof(prio) * 8U - PRIO_BM_DATA_WIDTH_LOG2));
    grpIndx = prio >> PRIO_BM_DATA_WIDTH_LOG2;
    bm->bit[grpIndx] &= ~PORT_PWR2(bitIndx);

    if (0U == bm->bit[grpIndx]) {                                               /* Is this the last one bit cleared in this group?          */
        bm->bitGrp &= ~PORT_PWR2(grpIndx);                                      /* Yes: then clear bit group indicator, too.                */
    }
#else
    bm->bit[0] &= ~PORT_PWR2(prio);
#endif
}

static PORT_C_INLINE uint_fast8_t prioBMGet(
    const prioBM_T *    bm) {

#if (1U != PRIO_BM_GRP_INDX)
    uint_fast8_t    grpIndx;
    uint_fast8_t    bitIndx;

    grpIndx = PORT_FIND_LAST_SET(bm->bitGrp);
    bitIndx = PORT_FIND_LAST_SET(bm->bit[grpIndx]);

    return ((grpIndx << PRIO_BM_DATA_WIDTH_LOG2) | bitIndx);
#else
    uint_fast8_t    bitIndx;

    bitIndx = PORT_FIND_LAST_SET(bm->bit[0]);

    return (bitIndx);
#endif
}

static PORT_C_INLINE bool_T prioBMIsEmpty(
    const prioBM_T *    bm) {

#if (1U != PRIO_BM_GRP_INDX)
    bool_T          ans;

    if (0U == bm->bitGrp) {
        ans = TRUE;
    } else {
        ans = FALSE;
    }

    return (ans);
#else
    bool_T          ans;

    if (0U == bm->bit[0]) {
        ans = TRUE;
    } else {
        ans = FALSE;
    }

    return (ans);
#endif
}


/*--  Scheduler functions  ---------------------------------------------------*/

static PORT_C_INLINE void schedInit(
    void) {

    esThdQInit(
        &gRdyQueue);                                                            /* Initialize basic thread queue structure                  */
    ((volatile esKernCtrl_T *)&gKernCtrl)->state = ES_KERN_INIT;
}

static PORT_C_INLINE void schedStart(
    void) {

    ES_CRITICAL_DECL();
    esThd_T * nthd;

    ES_CRITICAL_ENTER();
    nthd = esThdQFetchI(                                                        /* Get the highest priority thread                          */
        &gRdyQueue);
    ((esKernCtrl_T *)&gKernCtrl)->cthd  = nthd;
    ((esKernCtrl_T *)&gKernCtrl)->pthd  = nthd;
    ((esKernCtrl_T *)&gKernCtrl)->state = ES_KERN_RUN;
    ES_CRITICAL_EXIT();
}

#if (1U == CFG_SCHED_POWER_SAVE) || defined(__DOXYGEN__)
static PORT_C_INLINE void schedSleep(
    void) {

    ES_CRITICAL_DECL();

    ES_CRITICAL_ENTER();

    if (ES_KERN_SLEEP != gKernCtrl.state) {

        ((esKernCtrl_T *)&gKernCtrl)->state = ES_KERN_SLEEP;
# if (1U == CFG_SYSTMR_ADAPTIVE_MODE)
        vTmrImportPendI();                                                      /* Import any pending timers.                               */
        sysTmrDeactivateI();
# endif
        PORT_CRITICAL_EXIT_SLEEP_ENTER();
    } else {
        ES_CRITICAL_EXIT();
    }
}
#endif

#if (1U == CFG_SCHED_POWER_SAVE) || defined(__DOXYGEN__)
static PORT_C_INLINE void schedWakeUpI(
    void) {

    ((esKernCtrl_T *)&gKernCtrl)->state = ES_KERN_RUN;
# if (1U == CFG_SYSTMR_ADAPTIVE_MODE)
    sysTmrActivate();
# endif
}
#endif

static PORT_C_INLINE void schedRdyAddInitI(
    esThd_T *       thd) {

    if (NULL == gKernCtrl.pthd) {
        ((esKernCtrl_T *)&gKernCtrl)->pthd = thd;
    }
}

static PORT_C_INLINE void schedQmNextI(
    void) {

    esThd_T * nthd;
    esThd_T * cthd;

    cthd = gKernCtrl.cthd;
    nthd = esThdQFetchRotateI(                                                  /* Fetch the next thread and rotate this priority group     */
        &gRdyQueue,
        cthd->prio);

    if (cthd == gKernCtrl.pthd) {                                               /* If there is no any other thread pending for switching    */
        ((esKernCtrl_T *)&gKernCtrl)->pthd = nthd;                              /* Make the new thread pending                              */
    }
}

static PORT_C_INLINE void schedQmI(
    void) {

    if (ES_KERN_LOCK > gKernCtrl.state) {                                       /* Round-Robin is not enabled in kernel LOCK state          */
        esThd_T * cthd;

        cthd = gKernCtrl.cthd;                                                  /* Get the current thread                                   */

        if (!DLIST_IS_ENTRY_SINGLE(thdL, cthd)) {
            cthd->qCnt--;                                                       /* Decrement current thread time quantum                    */

            if (0U == cthd->qCnt) {
                cthd->qCnt = cthd->qRld;                                        /* Reload thread time quantum                               */
                schedQmNextI();
            }
        }
    }
}


/*--  System timer  ----------------------------------------------------------*/

static PORT_C_INLINE void sysTmrInit(
    void) {

    PORT_SYSTMR_INIT();
    PORT_SYSTMR_ENABLE();
    PORT_SYSTMR_ISR_ENABLE();
}

#if (1U == CFG_SYSTMR_ADAPTIVE_MODE) || defined(__DOXYGEN__)
static PORT_C_INLINE void sysTmrActivate(
    void) {

    if (0U == gSysTmr.ptick) {                                                  /* Normal wake up.                                          */

        if (0U != gSysTmr.vTmrArmed) {                                          /* System timer was enabled during sleep.                   */
            portSysTmrReg_T tmrVal;

            tmrVal = PORT_SYSTMR_GET_CVAL() % PORT_SYSTMR_ONE_TICK_VAL;
            PORT_SYSTMR_RLD(tmrVal);
        } else {                                                                /* System timer was disabled during sleep.                  */
            PORT_SYSTMR_RLD(PORT_SYSTMR_ONE_TICK_VAL);                          /* Reload macro will also re-enable the timer               */
            PORT_SYSTMR_ISR_ENABLE();
        }
    } else {                                                                    /* Preempted wake up, system timer was enabled during sleep.*/
        esVTmr_T * vTmr;
        portSysTmrReg_T tmrVal;

        vTmr = DLIST_ENTRY_NEXT(tmrL, &gVTmrArmed);
        tmrVal = (PORT_SYSTMR_GET_RVAL() - PORT_SYSTMR_GET_CVAL());
        vTmr->rtick -= tmrVal / PORT_SYSTMR_ONE_TICK_VAL;
        tmrVal %= PORT_SYSTMR_ONE_TICK_VAL;
        gSysTmr.ptick = 0U;
        PORT_SYSTMR_RLD(tmrVal);
    }
}
#endif

#if (1U == CFG_SYSTMR_ADAPTIVE_MODE) || defined(__DOXYGEN__)
static PORT_C_INLINE void sysTmrDeactivateI(
    void) {

    if (0U != gSysTmr.vTmrArmed) {                                              /* There is an armed timer: put system timer to sleep sleep.*/
        esVTmr_T * vTmr;

        vTmr = DLIST_ENTRY_NEXT(tmrL, &gVTmrArmed);
        vTmrSleep(
            vTmr->rtick);
    } else {                                                                    /* No virtual timer is armed: set system timer to disabled. */
        PORT_SYSTMR_ISR_DISABLE();
        PORT_SYSTMR_DISABLE();
    }
}
#endif


/*--  Timer  -----------------------------------------------------------------*/

#if (1U == CFG_SYSTMR_ADAPTIVE_MODE) || defined(__DOXYGEN__)
static PORT_C_INLINE void vTmrSleep(
    esTick_T        ticks) {

    portSysTmrReg_T sysTmrVal;

    gSysTmr.ptick = ticks;

    if (PORT_SYSTMR_MAX_TICKS_VAL < gSysTmr.ptick) {
        gSysTmr.ptick = PORT_SYSTMR_MAX_TICKS_VAL;
    }
    sysTmrVal = PORT_SYSTMR_ONE_TICK_VAL * gSysTmr.ptick;
    sysTmrVal += PORT_SYSTMR_GET_CVAL() % PORT_SYSTMR_ONE_TICK_VAL;

    if (PORT_SYSTMR_GET_RVAL() != sysTmrVal) {
        PORT_SYSTMR_RLD(sysTmrVal);
    }
}
#endif

static PORT_C_INLINE void vTmrEvaluateI(
    void) {

    if (0U != gSysTmr.vTmrArmed) {                                              /* There is an armed timer waiting.                         */
        esVTmr_T * vTmr;

        vTmr = DLIST_ENTRY_NEXT(tmrL, &gVTmrArmed);

#if (0U == CFG_SYSTMR_ADAPTIVE_MODE)
        --vTmr->rtick;
#elif (1U == CFG_SYSTMR_ADAPTIVE_MODE)
        if (0U == gSysTmr.ptick) {                                              /* Normal system tick.                                      */

            if (PORT_SYSTMR_ONE_TICK_VAL != PORT_SYSTMR_GET_RVAL()) {           /* If system timer is in adaptive mode switch it to fixed   */
                PORT_SYSTMR_RLD(PORT_SYSTMR_ONE_TICK_VAL);                      /* mode.                                                    */
            }
            --vTmr->rtick;
        } else {                                                                /* Low power tick.                                          */
            vTmr->rtick -= gSysTmr.ptick;                                       /* Substract pending ticks from current timer.              */

            if (0U != vTmr->rtick) {                                            /* If the timer still hasn't expired schedule another sleep */
                vTmrSleep(                                                      /* time period.                                             */
                    vTmr->rtick);
            }
        }
#endif

        if (0U == vTmr->rtick) {                                                /* A timer has expired, start kVTmr thread.                 */
            esSchedRdyAddI(
                &gKVTmr);
        }
    }

    if (0U != gSysTmr.vTmrPend) {                                               /* There is a timer pending, start kVTmr thread.            */

        if (NULL == gKVTmr.thdL.q) {
            esSchedRdyAddI(
                &gKVTmr);
        }
    }
}

static void vTmrAddArmed(
    esVTmr_T *      vTmr) {

    esVTmr_T *      tmp;
    esTick_T        tick;

    vTmr->tmrL.q = &gVTmrArmed;
    tmp = DLIST_ENTRY_NEXT(tmrL, &gVTmrArmed);
    tick = vTmr->rtick;

    while (tmp->rtick < tick) {
        tick -= tmp->rtick;
        tmp = DLIST_ENTRY_NEXT(tmrL, tmp);
    }
    vTmr->rtick = tick;
    DLIST_ENTRY_ADD_AFTER(tmrL, tmp, vTmr);

    if (&gVTmrArmed != tmp) {
        tmp->rtick -= vTmr->rtick;
    }
}

#if (1U == CFG_SYSTMR_ADAPTIVE_MODE) || defined(__DOXYGEN__)
static PORT_C_INLINE void vTmrImportPendI(
    void) {

    while (0U != gSysTmr.vTmrPend) {
        esVTmr_T * tmr;

        ++gSysTmr.vTmrArmed;
        --gSysTmr.vTmrPend;
        tmr = DLIST_ENTRY_NEXT(tmrL, &gVTmrPend);
        DLIST_ENTRY_RM(tmrL, tmr);
        vTmrAddArmed(
            tmr);
    }
}
#endif

static void vTmrImportPend(
    void) {

    ES_CRITICAL_DECL();

    ES_CRITICAL_ENTER();

    while (0U != gSysTmr.vTmrPend) {
        esVTmr_T * tmr;

        ++gSysTmr.vTmrArmed;
        --gSysTmr.vTmrPend;
        tmr = DLIST_ENTRY_NEXT(tmrL, &gVTmrPend);
        DLIST_ENTRY_RM(tmrL, tmr);
        ES_CRITICAL_EXIT();
        vTmrAddArmed(
            tmr);
        ES_CRITICAL_ENTER();
    }
    ES_CRITICAL_EXIT();
}


/*--  Kernel threads  --------------------------------------------------------*/

/* 1)       Kernel Virtual Timer thread must have the highest priority available.
 */
static void kVTmrInit(
    void) {

    static portStck_T kVTmrStck[ES_STCK_SIZE(PORT_KVTMR_STCK_SIZE)];            /* Virtual timer kernel thread stack.                       */

    esThdInit(
        &gKVTmr,
        kVTmr,
        NULL,
        kVTmrStck,
        sizeof(kVTmrStck),
        CFG_SCHED_PRIO_LVL - 1U);
}

/* 1)       This thread is just waiting continuously on thread semaphore and
 *          execute virtual timers callback functions if there are any available
 *          and then import pending virtual timers into armed linked list.
 */
static void kVTmr(
    void *          arg) {

    (void)arg;

    while (TRUE) {

        esThdWait();

        if (0U != gSysTmr.vTmrArmed) {                                          /* There is at least one armed timer.                       */
            esVTmr_T * tmr;

            tmr = DLIST_ENTRY_NEXT(tmrL, &gVTmrArmed);

            while (0U == tmr->rtick) {
                esVTmr_T * tmpTmr;

                --gSysTmr.vTmrArmed;
                DLIST_ENTRY_RM(tmrL, tmr);
                tmpTmr = tmr;
                tmr = DLIST_ENTRY_NEXT(tmrL, &gVTmrArmed);
                (* tmpTmr->fn)(tmpTmr->arg);
            }
        }
        vTmrImportPend();                                                       /* Import pending timers if there are any.                  */
    }
}

/* 1)       Kernel Idle thread must have the lowest priority.
 */
static void kIdleInit(
    void) {

    static portStck_T kIdleStck[ES_STCK_SIZE(PORT_KIDLE_STCK_SIZE)];            /* Idle kernel thread stack.                                */

    esThdInit(
        &gKIdle,
        kIdle,
        NULL,
        kIdleStck,
        sizeof(kIdleStck),
        0U);
}

/* 1)       Idle thread must be the only thread at this priority level.
 */
static void kIdle(
    void *          arg){

    (void)arg;

    while (TRUE) {

#if (1U == CFG_HOOK_PRE_IDLE)
    userPreIdle();
#endif

#if (0U == CFG_SCHED_POWER_SAVE)
        ES_CRITICAL_DECL();

        ES_CRITICAL_ENTER();
        schedQmNextI();
        esSchedYieldI();
        ES_CRITICAL_EXIT();
#elif (1U == CFG_SCHED_POWER_SAVE)
        schedSleep();
#endif

#if (1U == CFG_HOOK_IDLE_END)
    userIdlePost();
#endif
    }
}

/*===================================  GLOBAL PRIVATE FUNCTION DEFINITIONS  ==*/
/*====================================  GLOBAL PUBLIC FUNCTION DEFINITIONS  ==*/


/*--  Kernel General functions  ----------------------------------------------*/

void esKernInit(
    void) {

    ES_DBG_API_REQUIRE(ES_DBG_USAGE_FAILURE, ES_KERN_INACTIVE == gKernCtrl.state);

#if (1U == CFG_HOOK_PRE_KERN_INIT)
    userPreKernInit();
#endif
    PORT_INT_DISABLE();
    PORT_INIT_EARLY();
    sysTmrInit();
    schedInit();
    kIdleInit();
    kVTmrInit();
    PORT_INIT();

#if (1U == CFG_HOOK_POST_KERN_INIT)
    userPostKernInit();
#endif
}

/* 1)       Since this function will never return it is marked with `noreturn`
 *          attribute to allow for compiler optimizations.
 */
PORT_C_NORETURN void esKernStart(
    void) {

    ES_DBG_API_REQUIRE(ES_DBG_USAGE_FAILURE, ES_KERN_INIT == gKernCtrl.state);

#if (1U == CFG_HOOK_PRE_KERN_START)
    userPreKernStart();
#endif
    PORT_INIT_LATE();
    schedStart();                                                               /* Initialize scheduler data structures for multi-threading */
    PORT_THD_START();                                                           /* Start the first thread                                   */

    while (TRUE);
}

void esKernSysTmr(
    void) {

    ES_CRITICAL_DECL();

#if (1U == CFG_HOOK_PRE_SYSTMR_EVENT)
    userPreSysTmr();
#endif
    ES_CRITICAL_ENTER();
    vTmrEvaluateI();
    schedQmI();
    ES_CRITICAL_EXIT();
}

void esKernLockEnterI(
    void) {

    if (0U == gKernLockCnt) {
        ((esKernCtrl_T *)&gKernCtrl)->state |= SCHED_STATE_LOCK_MSK;
    }
    ++gKernLockCnt;
}

void esKernLockExitI(
    void) {

    --gKernLockCnt;

    if (0U == gKernLockCnt) {
        ((esKernCtrl_T *)&gKernCtrl)->state &= ~SCHED_STATE_LOCK_MSK;
        esSchedYieldI();
    }
}

void esKernLockEnter(
    void) {

    ES_CRITICAL_DECL();

    ES_CRITICAL_ENTER();
    esKernLockEnterI();
    ES_CRITICAL_EXIT();
}

void esKernLockExit(
    void) {

    ES_CRITICAL_DECL();

    ES_CRITICAL_ENTER();
    esKernLockExitI();
    ES_CRITICAL_EXIT();
}

void esKernIsrPrologueI(
    void) {

    ES_DBG_API_REQUIRE(ES_DBG_USAGE_FAILURE, ES_KERN_INIT > gKernCtrl.state);

    ((esKernCtrl_T *)&gKernCtrl)->state |= SCHED_STATE_INTSRV_MSK;
}

void esKernIsrEpilogueI(
    void) {

    ES_DBG_API_REQUIRE(ES_DBG_USAGE_FAILURE, ES_KERN_INIT > gKernCtrl.state);

    if (TRUE == PORT_ISR_IS_LAST()) {
        ((esKernCtrl_T *)&gKernCtrl)->state &= ~SCHED_STATE_INTSRV_MSK;
        esSchedYieldIsrI();
    }
}


/*--  Thread functions  ------------------------------------------------------*/

void esThdInit(
    esThd_T *       thd,
    void (* fn)(void *),
    void *          arg,
    portStck_T *    stck,
    size_t          stckSize,
    uint8_t         prio) {

	ES_CRITICAL_DECL();

    ES_DBG_API_REQUIRE(ES_DBG_USAGE_FAILURE, ES_KERN_INACTIVE > gKernCtrl.state);
    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL != thd);
    ES_DBG_API_REQUIRE(ES_DBG_OBJECT_NOT_VALID, THD_CONTRACT_SIGNATURE != thd->signature);
    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL != fn);
    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL != stck);
    ES_DBG_API_REQUIRE(ES_DBG_OUT_OF_RANGE, PORT_STCK_MINSIZE_VAL <= (stckSize * sizeof(portReg_T)));
    ES_DBG_API_REQUIRE(ES_DBG_OUT_OF_RANGE, CFG_SCHED_PRIO_LVL > prio);
    ES_DBG_API_REQUIRE(ES_DBG_OUT_OF_RANGE, ((&gKVTmr != thd) && ((CFG_SCHED_PRIO_LVL - 1U) > prio)) || (&gKVTmr == thd));
    ES_DBG_API_REQUIRE(ES_DBG_OUT_OF_RANGE, ((&gKIdle != thd) && (0U < prio)) || (&gKIdle == thd));

    thd->stck   = PORT_CTX_INIT(stck, stckSize, fn, arg);                       /* Make a fake thread stack.                                */
    thd->thdL.q = NULL;                                                         /* This thread is not in any thread queue.                  */
    DLIST_ENTRY_INIT(thdL, thd);
    thd->prio   = prio;                                                         /* Set the priority.                                        */
    thd->cprio  = prio;                                                         /* This is constant priority, it never changes.             */
    thd->qCnt   = CFG_SCHED_TIME_QUANTUM;
    thd->qRld   = CFG_SCHED_TIME_QUANTUM;

    ES_DBG_API_OBLIGATION(thd->signature = THD_CONTRACT_SIGNATURE);             /* Make thread structure valid.                             */

    ES_CRITICAL_ENTER();
    schedRdyAddInitI(
        thd);                                                                   /* Initialize thread before adding it to Ready Thread queue.*/
    esSchedRdyAddI(
        thd);                                                                   /* Add the thread to Ready Thread queue.                    */
    esSchedYieldI();                                                            /* Invoke the scheduler.                                    */
    ES_CRITICAL_EXIT();

#if (1U == CFG_HOOK_POST_THD_INIT)
    userPostThdInit();
#endif
}

void esThdTerm(
    esThd_T *       thd) {

    ES_CRITICAL_DECL();

    ES_DBG_API_REQUIRE(ES_DBG_USAGE_FAILURE, ES_KERN_INACTIVE > gKernCtrl.state);
    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL != thd);
    ES_DBG_API_REQUIRE(ES_DBG_OBJECT_NOT_VALID, THD_CONTRACT_SIGNATURE == thd->signature);
    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, (NULL == thd->thdL.q) || (&gRdyQueue == thd->thdL.q));

#if (1U == CFG_HOOK_PRE_THD_TERM)
    userPreThdTerm();
#endif
    ES_CRITICAL_ENTER();

    if (&gRdyQueue == thd->thdL.q) {
        esSchedRdyRmI(
            thd);
    } else if (NULL != thd->thdL.q) {
        esThdQRmI(
            thd->thdL.q,
            thd);
    }

    ES_DBG_API_OBLIGATION(thd->signature = ~THD_CONTRACT_SIGNATURE);            /* Mark the thread ID structure as invalid.                 */

    esSchedYieldI();
    ES_CRITICAL_EXIT();
}

void esThdSetPrioI(
    esThd_T *       thd,
    uint8_t         prio) {

    ES_DBG_API_REQUIRE(ES_DBG_USAGE_FAILURE, ES_KERN_INACTIVE > gKernCtrl.state);
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

        if (&gRdyQueue == thd->thdL.q) {                                        /* Is thread in ready thread queue?                         */

            if (thd->prio > gKernCtrl.pthd->prio) {                             /* If new prio is higher than the current prio notify the   */
                ((esKernCtrl_T *)&gKernCtrl)->pthd = thd;                       /* scheduler about new thread.                              */
            } else {
                ((esKernCtrl_T *)&gKernCtrl)->pthd = esThdQFetchI(
                    &gRdyQueue);
            }
        }
    }
}

/* 1)       Since this function can be called multiple times with the same
 *          thread then it needs to check if the thread is not already added in
 *          a queue.
 */
void esThdPostI(
    esThd_T *       thd) {

    if (NULL == thd->thdL.q) {
        esSchedRdyAddI(
            thd);
        esSchedYieldI();
    }
}

/* 1)       See notes for esThdPostI()
 */
void esThdPost(
    esThd_T *       thd) {

    ES_CRITICAL_DECL();

    ES_CRITICAL_ENTER();
    esThdPostI(
        thd);
    ES_CRITICAL_EXIT();
}

void esThdWaitI(
    void) {

    esSchedRdyRmI(
        esThdGetId());
    esSchedYieldI();
}

void esThdWait(
    void) {

    ES_CRITICAL_DECL();

    ES_CRITICAL_ENTER();
    esThdWaitI();
    ES_CRITICAL_EXIT();
}


/*--  Thread Queue functions  ------------------------------------------------*/

void esThdQInit(
    esThdQ_T *      thdQ) {

    uint_fast8_t    group;

    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL != thdQ);
    ES_DBG_API_REQUIRE(ES_DBG_OBJECT_NOT_VALID, THDQ_CONTRACT_SIGNATURE != thdQ->signature);

    prioBMInit(
        &thdQ->prioOcc);

    for (group = 0U; group < CFG_SCHED_PRIO_LVL; group++) {
        thdQ->grp[group].head = NULL;
        thdQ->grp[group].next  = NULL;
    }
    ES_DBG_API_OBLIGATION(thdQ->signature = THDQ_CONTRACT_SIGNATURE);
}

/* 1)       When API validation is not used then this function will become empty.
 */
void esThdQTerm(
    esThdQ_T *      thdQ) {

    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL != thdQ);
    ES_DBG_API_REQUIRE(ES_DBG_OBJECT_NOT_VALID, THDQ_CONTRACT_SIGNATURE == thdQ->signature);

    ES_DBG_API_OBLIGATION(thdQ->signature = ~THDQ_CONTRACT_SIGNATURE);

#if (0U == CFG_DBG_API_VALIDATION)
    (void)thdQ;                                                                 /* Prevent compiler warning about unused argument.          */
#endif
}

void esThdQAddI(
    esThdQ_T *      thdQ,
    esThd_T *       thd) {

    thdLSentinel_T * sentinel;

    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL != thdQ);
    ES_DBG_API_REQUIRE(ES_DBG_OBJECT_NOT_VALID, THDQ_CONTRACT_SIGNATURE == thdQ->signature);
    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL != thd);
    ES_DBG_API_REQUIRE(ES_DBG_OBJECT_NOT_VALID, THD_CONTRACT_SIGNATURE == thd->signature);
    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL == thd->thdL.q);
    ES_DBG_API_REQUIRE(ES_DBG_OUT_OF_RANGE, CFG_SCHED_PRIO_LVL >= thd->prio);

    sentinel = &(thdQ->grp[thd->prio]);                                         /* Get the sentinel from thread priority level.             */

    if (NULL == sentinel->head) {                                               /* Is thdL list empty?                                      */
        sentinel->head = thd;                                                   /* This thread becomes first in the list.                   */
        sentinel->next = thd;
        prioBMSet(
            &thdQ->prioOcc,
            thd->prio);                                                         /* Mark the priority group as used.                         */
    } else {
        DLIST_ENTRY_ADD_AFTER(thdL, sentinel->head, thd);                       /* Thread is added at the next of the list.                 */
    }
    thd->thdL.q = thdQ;                                                         /* Set the pointer to the thread queue being used.          */
}

void esThdQRmI(
    esThdQ_T *      thdQ,
    esThd_T *       thd) {

    thdLSentinel_T * sentinel;

    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL != thd);
    ES_DBG_API_REQUIRE(ES_DBG_OBJECT_NOT_VALID, THD_CONTRACT_SIGNATURE == thd->signature);
    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL != thdQ);
    ES_DBG_API_REQUIRE(ES_DBG_OBJECT_NOT_VALID, THDQ_CONTRACT_SIGNATURE == thdQ->signature);
    ES_DBG_API_REQUIRE(ES_DBG_USAGE_FAILURE, thdQ == thd->thdL.q);
    ES_DBG_API_REQUIRE(ES_DBG_OUT_OF_RANGE, CFG_SCHED_PRIO_LVL >= thd->prio);

    sentinel = &(thdQ->grp[thd->prio]);                                         /* Get the sentinel from thread priority level.             */

    if (DLIST_IS_ENTRY_LAST(thdL, thd)) {                                       /* Is this thread last one in the thdL list?                */
        sentinel->head = NULL;                                                  /* Make the list sentinel empty.                            */
        prioBMClear(
            &thdQ->prioOcc,
            thd->prio);                                                         /* Remove the mark since this group is not used.            */
    } else {                                                                    /* This thread is not the last one in the thdL list.        */

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

    thdLSentinel_T * sentinel;
    uint_fast8_t    prio;

    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL != thdQ);
    ES_DBG_API_REQUIRE(ES_DBG_OBJECT_NOT_VALID, THDQ_CONTRACT_SIGNATURE == thdQ->signature);
    ES_DBG_API_REQUIRE(ES_DBG_USAGE_FAILURE, FALSE == prioBMIsEmpty(&thdQ->prioOcc));

    prio = prioBMGet(
        &thdQ->prioOcc);                                                        /* Get the highest priority ready to run.                   */
    sentinel = (thdLSentinel_T *)&(thdQ->grp[prio]);                            /* Get the Group Head pointer for that priority.            */
                                                                                /* The type cast is needed to avoid compiler warnings.      */
    ES_DBG_API_ENSURE(ES_DBG_OBJECT_NOT_VALID, THD_CONTRACT_SIGNATURE == sentinel->next->signature);

    return (sentinel->next);
}

esThd_T * esThdQFetchRotateI(
    esThdQ_T *      thdQ,
    uint_fast8_t    prio) {

    thdLSentinel_T * sentinel;

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

    bool_T          ans;

    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL != thdQ);
    ES_DBG_API_REQUIRE(ES_DBG_OBJECT_NOT_VALID, THDQ_CONTRACT_SIGNATURE == thdQ->signature);

    ans = prioBMIsEmpty(
        &thdQ->prioOcc);

    return (ans);
}


/*--  Scheduler functions  ---------------------------------------------------*/

void esSchedRdyAddI(
    esThd_T *       thd) {

    ES_DBG_API_REQUIRE(ES_DBG_USAGE_FAILURE, ES_KERN_INACTIVE > gKernCtrl.state);
    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL != thd);
    ES_DBG_API_REQUIRE(ES_DBG_OBJECT_NOT_VALID, THD_CONTRACT_SIGNATURE == thd->signature);
    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL == thd->thdL.q);

    esThdQAddI(
        &gRdyQueue,
        thd);

    if (thd->prio > gKernCtrl.pthd->prio) {
        ((esKernCtrl_T *)&gKernCtrl)->pthd = thd;
    }
}

/* 1)       If this function is removing currently executed thread or the
 *          pending thread then the scheduler will be invoked to get new highest
 *          priority thread.
 */
void esSchedRdyRmI(
    esThd_T *       thd) {

    ES_DBG_API_REQUIRE(ES_DBG_USAGE_FAILURE, ES_KERN_INACTIVE > gKernCtrl.state);
    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL != thd);
    ES_DBG_API_REQUIRE(ES_DBG_OBJECT_NOT_VALID, THD_CONTRACT_SIGNATURE == thd->signature);
    ES_DBG_API_REQUIRE(ES_DBG_USAGE_FAILURE, &gRdyQueue == thd->thdL.q);

    esThdQRmI(
        &gRdyQueue,
        thd);

    if ((gKernCtrl.cthd == thd) || (gKernCtrl.pthd == thd)) {
        ((esKernCtrl_T *)&gKernCtrl)->pthd = esThdQFetchI(
            &gRdyQueue);                                                        /* Get new highest priority thread.                         */
    }
}

void esSchedYieldI(
    void) {

    ES_DBG_API_REQUIRE(ES_DBG_USAGE_FAILURE, ES_KERN_INACTIVE > gKernCtrl.state);

    if (gKernCtrl.cthd != gKernCtrl.pthd) {                                     /* Context switching is needed only when cthd and nthd are  */
                                                                                /* different.                                               */
        if (ES_KERN_RUN == gKernCtrl.state) {

#if (1U == CFG_HOOK_PRE_CTX_SW)
            userPreCtxSw(
                gKernCtrl.cthd,
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

    ES_DBG_API_REQUIRE(ES_DBG_USAGE_FAILURE, ES_KERN_INACTIVE > gKernCtrl.state);

    if (gKernCtrl.cthd != gKernCtrl.pthd) {

        if (ES_KERN_RUN == gKernCtrl.state) {

#if (1U == CFG_HOOK_PRE_CTX_SW)
        userPreCtxSw(
            gKernCtrl.cthd,
            gKernCtrl.pthd);
#endif
            PORT_CTX_SW_ISR();

#if (1U == CFG_SCHED_POWER_SAVE)
        } else if (ES_KERN_SLEEP == gKernCtrl.state) {
            schedWakeUpI();

# if (1U == CFG_HOOK_PRE_CTX_SW)
        userPreCtxSw(
            gKernCtrl.cthd,
            gKernCtrl.pthd);
# endif
            PORT_CTX_SW_ISR();
#endif
        }
    }
}


/*--  Timer functions  -------------------------------------------------------*/

void esVTmrInitI(
    esVTmr_T *      vTmr,
    esTick_T        tick,
    void (* fn)(void *),
    void *          arg) {

    ES_DBG_API_REQUIRE(ES_DBG_USAGE_FAILURE, ES_KERN_INACTIVE > gKernCtrl.state);
    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL != vTmr);
    ES_DBG_API_REQUIRE(ES_DBG_OBJECT_NOT_VALID, VTMR_CONTRACT_SIGNATURE != vTmr->signature);
    ES_DBG_API_REQUIRE(ES_DBG_OUT_OF_RANGE, 1U < tick);
    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL != fn);

    vTmr->rtick = tick - 1U;                                                    /* Timer thread requires one tick less than original value. */
    vTmr->fn    = fn;
    vTmr->arg   = arg;
    DLIST_ENTRY_ADD_AFTER(tmrL, &gVTmrPend, vTmr);
    ++gSysTmr.vTmrPend;

#if (1U == CFG_SYSTMR_ADAPTIVE_MODE)
    if (0U != gSysTmr.ptick) {                                                  /* If system is sleeping we need to wake up VTmt thread.    */
        esThdPostI(
            &gKVTmr);
    }
#endif
    ES_DBG_API_OBLIGATION(vTmr->signature = VTMR_CONTRACT_SIGNATURE);
}

void esVTmrInit(
    esVTmr_T *      vTmr,
    esTick_T        tick,
    void (* fn)(void *),
    void *          arg) {

    ES_CRITICAL_DECL();

    ES_CRITICAL_ENTER();
    esVTmrInitI(
        vTmr,
        tick,
        fn,
        arg);
    ES_CRITICAL_EXIT();
}

void esVTmrTermI(
    esVTmr_T *      vTmr) {

    ES_DBG_API_REQUIRE(ES_DBG_USAGE_FAILURE, ES_KERN_INACTIVE > gKernCtrl.state);
    ES_DBG_API_REQUIRE(ES_DBG_POINTER_NULL, NULL != vTmr);
    ES_DBG_API_REQUIRE(ES_DBG_OBJECT_NOT_VALID, VTMR_CONTRACT_SIGNATURE == vTmr->signature);

    ES_DBG_API_OBLIGATION(vTmr->signature = ~VTMR_CONTRACT_SIGNATURE);

    if (&gVTmrPend == vTmr->tmrL.q) {                                           /* A pending timer is being deleted.                        */
        DLIST_ENTRY_RM(tmrL, vTmr);
        --gSysTmr.vTmrPend;
    } else {                                                                    /* An armed timer is being deleted.                         */

#if   (0U == CFG_SYSTMR_ADAPTIVE_MODE)
        esVTmr_T *      nextVTmr;

        --gSysTmr.vTmrArmed;
        nextVTmr = DLIST_ENTRY_NEXT(tmrL, vTmr);

        if (&gVTmrArmed != nextVTmr) {
            nextVTmr->rtick += vTmr->rtick;
        }
        DLIST_ENTRY_RM(tmrL, vTmr);
#elif (1U == CFG_SYSTMR_ADAPTIVE_MODE)
        --gSysTmr.vTmrArmed;

        if ((0U != gSysTmr.ptick) &&
            (DLIST_ENTRY_NEXT(tmrL, &gVTmrArmed) == vTmr)) {                    /* System timer was sleeping and vTmr is the current timer. */
            DLIST_ENTRY_RM(tmrL, vTmr);

            if (0U != gSysTmr.vTmrArmed) {                                      /* The last timer is not being deleted: remaining time is   */
                esVTmr_T * nextVTmr;                                            /* calculated to add to next timer in list.                 */

                vTmr->rtick -= (PORT_SYSTMR_GET_RVAL() - PORT_SYSTMR_GET_CVAL()) / PORT_SYSTMR_ONE_TICK_VAL;
                nextVTmr = DLIST_ENTRY_NEXT(tmrL, &gVTmrArmed);
                nextVTmr->rtick += vTmr->rtick;
            }
            sysTmrDeactivateI();
        } else {
            esVTmr_T * nextVTmr;

            nextVTmr = DLIST_ENTRY_NEXT(tmrL, vTmr);

            if (&gVTmrArmed != nextVTmr) {
                nextVTmr->rtick += vTmr->rtick;
            }
            DLIST_ENTRY_RM(tmrL, vTmr);
        }
#endif
    }
}

void esVTmrTerm(
    esVTmr_T *      vTmr) {

    ES_CRITICAL_DECL();

    ES_CRITICAL_ENTER();
    esVTmrTermI(
        vTmr);
    ES_CRITICAL_EXIT();
}

void esVTmrDelay(
    esTick_T        tick) {

    esVTmr_T        vTmr;

    esVTmrInit(
        &vTmr,
        tick,
        (void (*)(void *))esThdPost,
        (void *)esThdGetId());
    esThdWait();
}

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of kernel.c
 ******************************************************************************/
