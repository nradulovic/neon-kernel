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

/**@brief       Scheduler is using system timer Quantum mask
 */
#define SYSTMR_SCHED_QM_MSK             (1U << 0)

/**@brief       User is using system timer Quantum mask
 */
#define SYSTMR_USR_QM_MSK               (1U << 1)

/**@brief       Enable/disable scheduler power savings mode
 */
#if (0U == CFG_SYSTMR_MODE) || defined(__DOXYGEN__)
# define SCHED_POWER_SAVE               0U
#else
# define SCHED_POWER_SAVE               1U
#endif

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

/**@brief       Get the previous entry
 */
#define DLIST_ENTRY_PREV(list, entry)                                           \
    (entry)->list.prev

/**@brief       Get the next entry
 */
#define DLIST_ENTRY_NEXT(list, entry)                                           \
    (entry)->list.next

/**@brief       Initialize entry
 */
#define DLIST_ENTRY_INIT(list, entry)\
    do {                                                                        \
        (entry)->list.next = (entry);                                           \
        (entry)->list.prev = (entry);                                           \
    } while (0U)

/**@brief       Add new @c entry after @c current entry
 */
#define DLIST_ENTRY_ADD_AFTER(list, current, entry)                             \
    do {                                                                        \
        (entry)->list.next = (current);                                         \
        (entry)->list.prev = (entry)->list.next->list.prev;                     \
        (entry)->list.next->list.prev = (entry);                                \
        (entry)->list.prev->list.next = (entry);                                \
    } while (0U)

/**@brief       Remove the @c entry from a list
 */
#define DLIST_ENTRY_RM(list, entry)                                             \
    do {                                                                        \
        (entry)->list.next->list.prev = (entry)->list.prev;                     \
        (entry)->list.prev->list.next = (entry)->list.next;                     \
    } while (0U)

/**@brief       System Timer kernel thread stack size
 */
#define KVTMR_STCK_SIZE                 PORT_STCK_SIZE(40U)

/**@brief       Idle kernel thread stack size
 */
#define KIDLE_STCK_SIZE                 PORT_STCK_SIZE(40U)

/*======================================================  LOCAL DATA TYPES  ==*/

/*------------------------------------------------------------------------*//**
 * @name        System Timer
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       System timer state enumeration
 */
enum sysTmrState {
    SYSTMR_ACTIVE,                                                              /**< @brief System timer is running.                        */
    SYSTMR_INACTIVE                                                             /**< @brief System timer is stopped.                        */
};

/**@brief       Main System Timer structure
 * @note        1) Member `tick` exists only if FIXED mode is selected. When
 *              this mode is selected then kernel supports time ticking tracking.
 * @note        2) When INHIBITED or ADAPTIVE mode is selected members `qm` and
 *              `state` are used to manage Quantum intervals.
 */
struct sysTmr {
    uint_fast16_t       vTmr;                                                   /**< @brief The number of virtual timers in system.         */
#if (0U != CFG_SYSTMR_MODE)
    esTick_T            rtick;
    portSysTmrReg_T     tmrVal;
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

/**@brief       Set scheduler data structures ready for multi-threading
 * @details     This function is called just before multi-threading has
 *              commenced.
 */
static PORT_C_INLINE void schedStart(
    void);

static PORT_C_INLINE void schedSleep(
    void);

static PORT_C_INLINE void schedWakeUpI(
    void);

/**@brief       Initialize scheduler ready structure during the thread add
 *              operation
 * @param       thd
 *              Pointer to the thread currently being initialized.
 * @details     Function will initialize scheduler structures during the init
 *              phase of the kernel.
 */
static void schedRdyAddInitI(
    esThd_T *       thd);

static void schedQmNextI(
    void);

/**@brief       Do the Quantum (Round-Robin) scheduling
 */
static void schedQmI(
    void);

/**@} *//*----------------------------------------------------------------*//**
 * @name        System timer
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Initialize system timer hardware
 */
static void sysTmrInit(
    void);

/**@brief       Try to deactivate system timer
 */
#if (0 != CFG_SYSTMR_MODE) || defined(__DOXYGEN__)
static void sysTmrTryDeactivate(
    void);
#endif

/**@brief       Try to activate system timer
 */
#if (0 != CFG_SYSTMR_MODE) || defined(__DOXYGEN__)
static void sysTmrActivate(
    void);
#endif

/**@} *//*----------------------------------------------------------------*//**
 * @name        Virtual Timer kernel thread
 * @{ *//*--------------------------------------------------------------------*/

static void vTmrImport(
    void);

/**@brief       Add a virtual timer into sorted list
 * @param       list
 *              List: pointer to sorted list
 * @param       tmr
 *              Virtual timer: pointer to virtual timer to add
 */
static void vTmrListAddSort(
    esVTmr_T *       list,
    esVTmr_T *       vTmr);

/**@brief       Initialization of Virtual Timer kernel thread
 */
static void kVTmrInit(
    void);

/**@brief       Virtual Timer thread code
 * @param       arg
 *              Argument: thread does not use argument
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
#if (0U != CFG_SYSTMR_MODE)
    1U,
    PORT_SYSTMR_ONE_TICK_VAL
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
#else
   UINT_FAST32_MAX,
#endif
   NULL,
   NULL,
#if (1U == CFG_API_VALIDATION)
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
#if (1U == CFG_API_VALIDATION)
   VTMR_CONTRACT_SIGNATURE
#endif
};

/**@brief       Virtual timer thread ID
 */
static esThd_T gKVTmrId;

/**@brief       Virtual timer kernel thread stack
 */
static portStck_T gKVTmrStck[KVTMR_STCK_SIZE];

/**@} *//*----------------------------------------------------------------*//**
 * @name        Idle kernel thread
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Idle thread ID
 */
static esThd_T gKIdleId;

/**@brief       Idle kernel thread stack
 */
static portStck_T gKIdleStck[KIDLE_STCK_SIZE];

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

static PORT_C_INLINE void schedSleep(
    void) {

#if (1U == SCHED_POWER_SAVE)
    ES_CRITICAL_DECL();

    if (ES_KERN_SLEEP != gKernCtrl.state) {
        ES_CRITICAL_ENTER();

        if (DLIST_IS_ENTRY_SINGLE(thdL, gKernCtrl.cthd)) {
            ((esKernCtrl_T *)&gKernCtrl)->state = ES_KERN_SLEEP;
            sysTmrTryDeactivate();
            PORT_CRITICAL_EXIT_SLEEP_ENTER();
        } else {
            schedQmNextI();
            esSchedYieldI();
            ES_CRITICAL_EXIT();
        }
    }
#else
    ES_CRITICAL_DECL();

    ES_CRITICAL_ENTER();
    schedQmNextI();
    esSchedYieldI();
    ES_CRITICAL_EXIT();
#endif
}

static PORT_C_INLINE void schedWakeUpI(
    void) {

    ((esKernCtrl_T *)&gKernCtrl)->state = ES_KERN_RUN;
    sysTmrActivate();
}

static PORT_C_INLINE void schedRdyAddInitI(
    esThd_T *       thd) {

    if (NULL == gKernCtrl.pthd) {
        ((esKernCtrl_T *)&gKernCtrl)->pthd = thd;
    }
}

static void schedQmNextI(
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

static void schedQmI(
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

static void sysTmrInit(
    void) {

    PORT_SYSTMR_INIT();
    PORT_SYSTMR_ENABLE();
    PORT_SYSTMR_ISR_ENABLE();
}

#define SYSTMR_KEEPBACK_VAL            1U

static void sysTmrActivate(
    void) {
#if (0U == CFG_SYSTMR_MODE) || defined(__DOXYGEN__)
#elif (1U == CFG_SYSTMR_MODE)
    portSysTmrReg_T tmrVal;

    tmrVal = PORT_SYSTMR_GET();

    if (tmrVal < (gSysTmr.tmrVal - (SYSTMR_KEEPBACK_VAL * PORT_SYSTMR_ONE_TICK_VAL))) {
        /*
         * Preempted timer activation
         */
        gSysTmr.rtick = tmrVal / PORT_SYSTMR_ONE_TICK_VAL;
    }
    PORT_SYSTMR_ACTV();
#endif
}

static void sysTmrTryDeactivate(
    void) {

#if (0U == CFG_SYSTMR_MODE) || defined(__DOXYGEN__)
#elif (1U == CFG_SYSTMR_MODE)
    if (0U == gSysTmr.vTmr) {
        PORT_SYSTMR_ISR_DISABLE();
        PORT_SYSTMR_DISABLE();
    } else {
        esVTmr_T * vTmr;

        vTmrImport();
        vTmr = DLIST_ENTRY_NEXT(tmrL, &gVTmrArmed);

        if ((PORT_SYSTMR_MAX_TICKS_VAL - SYSTMR_KEEPBACK_VAL) < vTmr->rtick) {
            gSysTmr.rtick = PORT_SYSTMR_MAX_TICKS_VAL - SYSTMR_KEEPBACK_VAL;
            gSysTmr.tmrVal = (PORT_SYSTMR_MAX_TICKS_VAL - SYSTMR_KEEPBACK_VAL) * PORT_SYSTMR_ONE_TICK_VAL;
        } else {
            gSysTmr.rtick = vTmr->rtick;
            gSysTmr.tmrVal = PORT_SYSTMR_ONE_TICK_VAL * gSysTmr.rtick;
        }
        PORT_SYSTMR_DACTV(gSysTmr.tmrVal);
    }
#endif
}


/*--  Timer  -----------------------------------------------------------------*/

static void vTmrExecHandlers(
    void) {

    if (!DLIST_IS_ENTRY_LAST(tmrL, &gVTmrArmed)) {
        esVTmr_T * tmr;

        tmr = DLIST_ENTRY_NEXT(tmrL, &gVTmrArmed);

#if (0U == CFG_SYSTMR_MODE)
        --tmr->rtick;
#elif (1U == CFG_SYSTMR_MODE)
        tmr->rtick -= gSysTmr.rtick;
        gSysTmr.rtick = 1U;
#endif

        while (0U == tmr->rtick) {
            --gSysTmr.vTmr;
            DLIST_ENTRY_RM(tmrL, tmr);
            (* tmr->fn)(tmr->arg);
            tmr = DLIST_ENTRY_NEXT(tmrL, &gVTmrArmed);
        }
    }
}

static void vTmrListAddSort(
    esVTmr_T *      list,
    esVTmr_T *      vTmr) {

    esVTmr_T *      tmp;
    esTick_T        tick;

    vTmr->tmrL.q = list;
    tmp = DLIST_ENTRY_NEXT(tmrL, list);
    tick = vTmr->rtick;

    while (tmp->rtick < tick) {
        tick -= tmp->rtick;
        tmp = DLIST_ENTRY_NEXT(tmrL, tmp);
    }
    vTmr->rtick = tick;
    DLIST_ENTRY_ADD_AFTER(tmrL, tmp, vTmr);

    if (list != tmp) {
        tmp->rtick -= vTmr->rtick;
    }
}

static void vTmrImport(
    void) {

    ES_CRITICAL_DECL();

    ES_CRITICAL_ENTER();
    while (!DLIST_IS_ENTRY_LAST(tmrL, &gVTmrPend)) {
        esVTmr_T * tmr;

        tmr = DLIST_ENTRY_NEXT(tmrL, &gVTmrPend);
        DLIST_ENTRY_RM(tmrL, tmr);
        ES_CRITICAL_EXIT();
        vTmrListAddSort(
            &gVTmrArmed,
            tmr);
        ES_CRITICAL_ENTER();
    }
    ES_CRITICAL_EXIT();
}

/*--  Kernel threads  --------------------------------------------------------*/

static void kVTmrInit(
    void) {

    esThdInit(
        &gKVTmrId,
        kVTmr,
        NULL,
        gKVTmrStck,
        sizeof(gKVTmrStck),
        CFG_SCHED_PRIO_LVL - 1U);
}

static void kVTmr(
    void *          arg) {

    (void)arg;

    while (TRUE) {

        esThdWait();
        vTmrExecHandlers();
        vTmrImport();
    }
}

static void kIdleInit(
    void) {

    esThdInit(
        &gKIdleId,
        kIdle,
        NULL,
        gKIdleStck,
        sizeof(gKIdleStck),
        0U);
    gKIdleId.qCnt = 1U;
    gKIdleId.qRld = 1U;
}

static void kIdle(
    void *          arg){

    (void)arg;

    while (TRUE) {

#if (1U == CFG_HOOK_IDLE_BEGIN)
#endif
        schedSleep();
#if (1U == CFG_HOOK_IDLE_END)
#endif
    }
}

/*===================================  GLOBAL PRIVATE FUNCTION DEFINITIONS  ==*/
/*====================================  GLOBAL PUBLIC FUNCTION DEFINITIONS  ==*/


/*--  Kernel General functions  ----------------------------------------------*/

void esKernInit(
    void) {

    ES_API_REQUIRE(ES_KERN_INACTIVE == gKernCtrl.state);

#if (1U == CFG_HOOK_KERN_INIT)
    userKernInit();
#endif
    PORT_INT_DISABLE();
    PORT_INIT_EARLY();
    sysTmrInit();
    schedInit();
    kIdleInit();
    kVTmrInit();
    PORT_INIT();
}

/* 1)       Since this function will never return it is marked with `noreturn`
 *          attribute to allow compiler optimizations.
 */
PORT_C_NORETURN void esKernStart(
    void) {

    ES_API_REQUIRE(ES_KERN_INIT == gKernCtrl.state);
    ES_API_REQUIRE(FALSE == esThdQIsEmpty(&gRdyQueue));

#if (1U == CFG_HOOK_KERN_START)
    userKernStart();
#endif
    PORT_INIT_LATE();
    schedStart();                                                               /* Initialize scheduler data structures for multi-threading */
    PORT_THD_START();                                                           /* Start the first thread                                   */

    while (TRUE) {
        ;
    }
}

void esKernSysTmrI(
    void) {

#if (1U == CFG_HOOK_SYSTMR_EVENT)
    userSysTmr();
#endif

    if (0U != gSysTmr.vTmr) {

        if (1U != gSysTmr.rtick) {
            PORT_SYSTMR_ACTV(gSysTmr.rtick);
        }
        esSchedRdyAddI(
            &gKVTmrId);
    }
    schedQmI();
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
    }
    esSchedYieldI();
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

    ES_API_REQUIRE(ES_KERN_INIT > gKernCtrl.state);

    ((esKernCtrl_T *)&gKernCtrl)->state |= SCHED_STATE_INTSRV_MSK;
}

void esKernIsrEpilogueI(
    void) {

    ES_API_REQUIRE(ES_KERN_INIT > gKernCtrl.state);

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

    ES_API_REQUIRE(ES_KERN_INACTIVE > gKernCtrl.state);
    ES_API_REQUIRE(NULL != thd);
    ES_API_REQUIRE(NULL != fn);
    ES_API_REQUIRE(NULL != stck);
    ES_API_REQUIRE(PORT_STCK_MINSIZE_VAL <= (stckSize * sizeof(portReg_T)));
    ES_API_REQUIRE(CFG_SCHED_PRIO_LVL >= prio);

    thd->stck   = PORT_CTX_INIT(stck, stckSize, fn, arg);                     /* Make a fake thread stack.                                */
    thd->thdL.q = NULL;                                                         /* This thread is not in any thread queue.                  */
    DLIST_ENTRY_INIT(thdL, thd);
    thd->prio   = prio;                                                         /* Set the priority.                                        */
    thd->cprio  = prio;                                                         /* This is constant priority, it never changes.             */
    thd->qCnt   = CFG_SCHED_TIME_QUANTUM;
    thd->qRld   = CFG_SCHED_TIME_QUANTUM;
    ES_API_OBLIGATION(thd->signature = THD_CONTRACT_SIGNATURE);                 /* Make thread structure valid.                             */
    ES_CRITICAL_ENTER();
    schedRdyAddInitI(
        thd);                                                                   /* Initialize thread before adding it to Ready Thread queue.*/
    esSchedRdyAddI(
        thd);                                                                   /* Add the thread to Ready Thread queue.                    */
    esSchedYieldI();                                                            /* Invoke the scheduler.                                    */
    ES_CRITICAL_EXIT();

#if (1U == CFG_HOOK_THD_INIT_END)
    userThdInitEnd();
#endif
}

void esThdTerm(
    esThd_T *       thd) {

    ES_CRITICAL_DECL();

    ES_API_REQUIRE(ES_KERN_INACTIVE > gKernCtrl.state);
    ES_API_REQUIRE(NULL != thd);
    ES_API_REQUIRE(THD_CONTRACT_SIGNATURE == thd->signature);
    ES_API_REQUIRE((NULL == thd->thdL.q) || (&gRdyQueue == thd->thdL.q));

#if (1U == CFG_HOOK_THD_TERM)
    userThdTerm();
#endif
    ES_CRITICAL_ENTER();

    if (&gRdyQueue == thd->thdL.q) {
        esSchedRdyRmI(
            thd);
    }
    ES_API_OBLIGATION(thd->signature = 0U);                                     /* Mark the thread ID structure as invalid.                 */
    esSchedYieldI();
    ES_CRITICAL_EXIT();
}

void esThdSetPrioI(
    esThd_T *       thd,
    uint8_t         prio) {

    /*
     * TODO: This function should be altered to take into account the case when
     * a thread is not in any queue (sleeping).
     */

    ES_API_REQUIRE(ES_KERN_INACTIVE > gKernCtrl.state);
    ES_API_REQUIRE(NULL != thd);
    ES_API_REQUIRE(THD_CONTRACT_SIGNATURE == thd->signature);
    ES_API_REQUIRE(CFG_SCHED_PRIO_LVL >= prio);

    if (prio >= thd->prio) {                                                    /* If new prio is higher than we may need to notify sched.  */
        esThdQRmI(
            thd->thdL.q,
            thd);
        thd->prio = prio;
        esThdQAddI(
            thd->thdL.q,
            thd);

        if (&gRdyQueue == thd->thdL.q) {                                        /* If thread is actually in ready thread queue              */

            if (thd->prio > gKernCtrl.pthd->prio) {                             /* If new prio is higher than the current prio              */
                ((esKernCtrl_T *)&gKernCtrl)->pthd = thd;              /* Notify scheduler about new thread                        */
            }
        }
    } else {
        esThdQRmI(
            thd->thdL.q,
            thd);
        thd->prio = prio;
        esThdQAddI(
            thd->thdL.q,
            thd);

        if (&gRdyQueue == thd->thdL.q) {
            ((esKernCtrl_T *)&gKernCtrl)->pthd = esThdQFetchI(
                &gRdyQueue);
        }
    }
}

/* 1)       Since this function can be called multiple times with the same
 *          thread then it needs to check if the thread is not already in a
 *          queue.
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

    uint8_t         group;

    ES_API_REQUIRE(NULL != thdQ);

    prioBMInit(
        &thdQ->prioOcc);

    for (group = 0U; group < CFG_SCHED_PRIO_LVL; group++) {
        thdQ->grp[group].head = NULL;
        thdQ->grp[group].next  = NULL;
    }
    ES_API_OBLIGATION(thdQ->signature = THDQ_CONTRACT_SIGNATURE);
}

void esThdQAddI(
    esThdQ_T *      thdQ,
    esThd_T *       thd) {

    thdLSentinel_T * sentinel;

    ES_API_REQUIRE(NULL != thdQ);
    ES_API_REQUIRE(THDQ_CONTRACT_SIGNATURE == thdQ->signature);
    ES_API_REQUIRE(NULL != thd);
    ES_API_REQUIRE(THD_CONTRACT_SIGNATURE == thd->signature);
    ES_API_REQUIRE(NULL == thd->thdL.q);
    ES_API_REQUIRE(CFG_SCHED_PRIO_LVL >= thd->prio);

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

    ES_API_REQUIRE(NULL != thd);
    ES_API_REQUIRE(THD_CONTRACT_SIGNATURE == thd->signature);
    ES_API_REQUIRE(NULL != thdQ);
    ES_API_REQUIRE(THDQ_CONTRACT_SIGNATURE == thdQ->signature);
    ES_API_REQUIRE(thdQ == thd->thdL.q);
    ES_API_REQUIRE(CFG_SCHED_PRIO_LVL >= thd->prio);

    sentinel = &(thdQ->grp[thd->prio]);                                         /* Get the sentinel from thread priority level.             */

    if (DLIST_IS_ENTRY_LAST(thdL, thd)) {                                       /* Is this thread last one in the thdL list?                */
        sentinel->head = NULL;                                                  /* Make the list empty.                                     */
        prioBMClear(
            &thdQ->prioOcc,
            thd->prio);                                                         /* Remove the mark since this group is not used.            */
    } else {                                                                    /* This thread is not the last one in the thdL list.        */

        if (sentinel->head == thd) {                                            /* In case we are removing thread from the beginning of the */
            sentinel->head = DLIST_ENTRY_NEXT(thdL, thd);                       /* list we need to advance head to point to the next one in */
        }                                                                       /* the list.                                                */

        if (sentinel->next == thd) {                                            /* In case we are removing thread from the end of the list  */
            sentinel->next = DLIST_ENTRY_NEXT(thdL, thd);                       /* we need to move next to point to a next one in the list. */
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

    ES_API_REQUIRE(NULL != thdQ);
    ES_API_REQUIRE(THDQ_CONTRACT_SIGNATURE == thdQ->signature);
    ES_API_REQUIRE(FALSE == prioBMIsEmpty(&thdQ->prioOcc));

    prio = prioBMGet(
        &thdQ->prioOcc);                                                        /* Get the highest priority ready to run.                   */
    sentinel = (thdLSentinel_T *)&(thdQ->grp[prio]);                            /* Get the Group Head pointer for that priority.            */
                                                                                /* The type cast is needed to avoid compiler warnings.      */
    return (sentinel->next);
}

esThd_T * esThdQFetchRotateI(
    esThdQ_T *      thdQ,
    uint_fast8_t    prio) {

    thdLSentinel_T * sentinel;

    ES_API_REQUIRE(NULL != thdQ);
    ES_API_REQUIRE(THDQ_CONTRACT_SIGNATURE == thdQ->signature);
    ES_API_REQUIRE(CFG_SCHED_PRIO_LVL >= prio);
    ES_API_REQUIRE(NULL != thdQ->grp[prio].next);

    sentinel = &(thdQ->grp[prio]);                                              /* Get the Group Head pointer from thread priority.         */
    sentinel->next = DLIST_ENTRY_NEXT(thdL, sentinel->next);

    return (sentinel->next);
}

bool_T esThdQIsEmpty(
    const esThdQ_T *    thdQ) {

    bool_T ans;

    ES_API_REQUIRE(NULL != thdQ);
    ES_API_REQUIRE(THDQ_CONTRACT_SIGNATURE == thdQ->signature);

    ans = prioBMIsEmpty(
        &thdQ->prioOcc);

    return (ans);
}


/*--  Scheduler functions  ---------------------------------------------------*/

void esSchedRdyAddI(
    esThd_T *       thd) {

    ES_API_REQUIRE(ES_KERN_INACTIVE > gKernCtrl.state);
    ES_API_REQUIRE(NULL != thd);
    ES_API_REQUIRE(THD_CONTRACT_SIGNATURE == thd->signature);
    ES_API_REQUIRE(NULL == thd->thdL.q);

    esThdQAddI(
        &gRdyQueue,
        thd);

    if (thd->prio > gKernCtrl.pthd->prio) {
        ((esKernCtrl_T *)&gKernCtrl)->pthd = thd;
    }
}

/* 1)       If this function is removing current thread or pending thread then
 *          the scheduler will be invoked to get new highest priority thread.
 */
void esSchedRdyRmI(
    esThd_T *       thd) {

    ES_API_REQUIRE(ES_KERN_INACTIVE > gKernCtrl.state);
    ES_API_REQUIRE(NULL != thd);
    ES_API_REQUIRE(THD_CONTRACT_SIGNATURE == thd->signature);
    ES_API_REQUIRE(&gRdyQueue == thd->thdL.q);

    esThdQRmI(
        &gRdyQueue,
        thd);

    if ((gKernCtrl.cthd == thd) || (gKernCtrl.pthd == thd)) {
        ((esKernCtrl_T *)&gKernCtrl)->pthd = esThdQFetchI(
            &gRdyQueue);
    }
}

/* 1)       The scheduler must always evaluate Qm regardless of the need to do
 *          context switching. Therefore schedQmEvaluateI() must be called
 *          before condition: `(newThd != gKernCtrl.cthd)`.
 */
void esSchedYieldI(
    void) {

    ES_API_REQUIRE(ES_KERN_INACTIVE > gKernCtrl.state);

    if (gKernCtrl.cthd != gKernCtrl.pthd) {                                     /* Context switching is needed only when cthd and nthd are  */
                                                                                /* different.                                               */
        if (ES_KERN_RUN == gKernCtrl.state) {

#if (1U == CFG_HOOK_CTX_SW)
            userCtxSw(
                gKernCtrl.cthd,
                newThd);
#endif
            PORT_CTX_SW();
        }
    }
}

/* 1)       See notes for esSchedYieldI()
 * 2)       This function is similar to esSchedYieldI() except it calls
 *          context switching macro for ISR and can wake up scheduler after
 *          idle sleep.
 */
void esSchedYieldIsrI(
    void) {

    ES_API_REQUIRE(ES_KERN_INACTIVE > gKernCtrl.state);

    if (gKernCtrl.cthd != gKernCtrl.pthd) {

        if (ES_KERN_RUN == gKernCtrl.state) {

#if (1U == CFG_HOOK_CTX_SW)
        userCtxSw(
            gKernCtrl.cthd,
            gKernCtrl.pthd);
#endif
            PORT_CTX_SW_ISR();

#if (1U == SCHED_POWER_SAVE)
        } else if (ES_KERN_SLEEP == gKernCtrl.state) {
            schedWakeUpI();

# if (1U == CFG_HOOK_CTX_SW)
        userCtxSw(
            gKernCtrl.cthd,
            gKernCtrl.pthd);
# endif
            PORT_CTX_SW_ISR();
#endif
        }
    }
}


/*--  System timer functions  ------------------------------------------------*/

/*--  Timer functions  -------------------------------------------------------*/

void esVTmrInitI(
    esVTmr_T *      vTmr,
    esTick_T        tick,
    void (* fn)(void *),
    void *          arg) {

    ES_API_REQUIRE(ES_KERN_INACTIVE > gKernCtrl.state);
    ES_API_REQUIRE(NULL != vTmr);
    ES_API_REQUIRE(1U < tick);
    ES_API_REQUIRE(NULL != fn);

    vTmr->rtick = tick;
    vTmr->fn    = fn;
    vTmr->arg   = arg;
    DLIST_ENTRY_ADD_AFTER(tmrL, &gVTmrPend, vTmr);
    ++gSysTmr.vTmr;

    ES_API_OBLIGATION(vTmr->signature = VTMR_CONTRACT_SIGNATURE);
}

void esVTmrInit(
    esVTmr_T *      vTmr,
    esTick_T        tick,
    void (* fn)(void *),
    void *          arg) {

    ES_CRITICAL_DECL();

    ES_API_REQUIRE(ES_KERN_INACTIVE > gKernCtrl.state);
    ES_API_REQUIRE(NULL != vTmr);
    ES_API_REQUIRE(1U < tick);
    ES_API_REQUIRE(NULL != fn);

    vTmr->rtick = tick - 1U;                                                    /* Timer thread requires one tick less than original value. */
    vTmr->fn    = fn;
    vTmr->arg   = arg;
    ES_API_OBLIGATION(vTmr->signature = VTMR_CONTRACT_SIGNATURE);
    vTmr->tmrL.q = &gVTmrPend;
    ES_CRITICAL_ENTER();
    DLIST_ENTRY_ADD_AFTER(tmrL, &gVTmrPend, vTmr);
    ++gSysTmr.vTmr;
    ES_CRITICAL_EXIT();
}

void esVTmrTerm(
    esVTmr_T *      vTmr) {

    ES_CRITICAL_DECL();

    ES_API_REQUIRE(ES_KERN_INACTIVE > gKernCtrl.state);
    ES_API_REQUIRE(NULL != vTmr);
    ES_API_REQUIRE(VTMR_CONTRACT_SIGNATURE == vTmr->signature);

    ES_CRITICAL_ENTER();
    ES_API_OBLIGATION(vTmr->signature = 0U);
    --gSysTmr.vTmr;

    if (&gVTmrPend == vTmr->tmrL.q) {
        DLIST_ENTRY_RM(tmrL, vTmr);
    } else {
        esVTmr_T *      nextVTmr;

        ES_CRITICAL_EXIT_LOCK_ENTER();
        nextVTmr = DLIST_ENTRY_NEXT(tmrL, vTmr);
        nextVTmr->rtick += vTmr->rtick;
        DLIST_ENTRY_RM(tmrL, vTmr);
        ES_CRITICAL_ENTER_LOCK_EXIT();
    }
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
