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
 * @author      Nenad Radulovic
 * @brief       Implementation of port independent code
 * @addtogroup  kern_impl
 *********************************************************************//** @{ */

/*=========================================================  INCLUDE FILES  ==*/
#include "kernel.h"

/*=========================================================  LOCAL MACRO's  ==*/

/**@brief       Priority Bit Map log base 2:
 *              <code>log2(PORT_DATA_WIDTH_VAL)</code>
 */
#define PRIO_BM_DATA_WIDTH_LOG2                                                 \
    (PORT_DATA_WIDTH_VAL <   2 ? 0 :                                                \
     (PORT_DATA_WIDTH_VAL <   4 ? 1 :                                               \
      (PORT_DATA_WIDTH_VAL <   8 ? 2 :                                              \
       (PORT_DATA_WIDTH_VAL <  16 ? 3 :                                             \
        (PORT_DATA_WIDTH_VAL <  32 ? 4 :                                            \
         (PORT_DATA_WIDTH_VAL <  64 ? 5 :                                           \
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
#define KTMR_STCK_SIZE                  PORT_STCK_SIZE(40U)

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
    SYSTMR_ACTIVE,                                                              /**< @brief System timer is running.                        *///!< SYSTMR_ACTIVE
    SYSTMR_INACTIVE                                                             /**< @brief System timer is stopped.                        *///!< SYSTMR_INACTIVE
};

/**@brief       Main System Timer structure
 */
struct sysTmr {
    esTick_T            tick;                                                   /**< @brief Current system tick counter                     */
    uint_fast16_t       tmr;                                                    /**< @brief The number of timers in system                  */
#if (0U != CFG_SYSTMR_MODE)
    uint_fast8_t        qm;                                                     /**< @brief Number of system timer quantum users.           */
    enum sysTmrState    state;                                                  /**< @brief Current state of system timer.                  */
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

/**@brief       Initialize scheduler ready structure during the thread add
 *              operation
 * @param       thd
 *              Pointer to the thread currently being initialized.
 * @details     Function will initialize scheduler structures during the init
 *              phase of the kernel.
 */
static void schedRdyAddInitI(
    esThd_T *       thd);

/**@brief       Do the Quantum (Round-Robin) scheduling
 */
static void schedQmI(
    void);

/**@brief       Activate system timer Quantum mode
 */
#if (1U == SCHED_POWER_SAVE) || defined(__DOXYGEN__)
static void schedQmActivate(
    void);
#endif

/**@brief       Deactivate system timer Quantum mode
 */
#if (1U == SCHED_POWER_SAVE) || defined(__DOXYGEN__)
static void schedQmDeactivate(
    void);
#endif

/**@brief       Evaluate if the system timer Quantum mode is needed
 * @param       thd
 *              Pointer to the thread which is ready to be executed
 */
#if (1U == SCHED_POWER_SAVE) || defined(__DOXYGEN__)
static void schedQmEvaluateI(
    esThd_T *       thd);
#endif

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
static void sysTmrTryActivate(
    void);
#endif

/**@} *//*----------------------------------------------------------------*//**
 * @name        System timer kernel thread
 * @{ *//*--------------------------------------------------------------------*/

static void tmrListAddSort(
    esTmr_T *       list,
    esTmr_T *       tmr);

/**@brief       Initialization of System Timer Thread
 */
static void kTmrInit(
    void);

/**@brief       System timer thread code
 * @param       arg
 *              NO ARGUMENTS - thread does not use argument
 */
static void kTmr(
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
 *              NO ARGUMENTS - thread does not use argument
 */
static void kIdle(
    void *          arg);

/**@} *//*--------------------------------------------------------------------*/
/*=======================================================  LOCAL VARIABLES  ==*/

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
#if (0U != CFG_SYSTMR_MODE)
    0U,
    SYSTMR_INACTIVE
#endif
};

/**@brief       Waiting list of timers to expire
 */
static esTmr_T gTmrWait = {
   {    &gTmrWait,
        &gTmrWait
   },

#if (0U == CFG_SYSTMR_TICK_TYPE)
   UINT_FAST8_MAX,
#elif (1U == CFG_SYSTMR_TICK_TYPE)
   UINT_FAST16_MAX,
#else
   UINT_FAST32_MAX,
#endif
   NULL,
   NULL
};

/**@brief       Timers pending to be inserted in waiting list
 */
static esTmr_T gTmrPend = {
   {    &gTmrPend,
        &gTmrPend
   },
   0U,
   NULL,
   NULL
};

/**@brief       System timer thread Id
 */
static esThd_T gKTmrId;

/**@brief       System timer thread stack
 */
static portStck_T gKTmrStck[KTMR_STCK_SIZE];

/**@} *//*----------------------------------------------------------------*//**
 * @name        Idle kernel thread
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Idle thread Id
 */
static esThd_T gKIdleId;

/**@brief       Idle thread stack
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

    PORT_CRITICAL_DECL();
    esThd_T * nthd;

    PORT_CRITICAL_ENTER();
    nthd = esThdQFetchFirstI(                                                   /* Get the highest priority thread                          */
        &gRdyQueue);

#if (1U == SCHED_POWER_SAVE)
    schedQmEvaluateI(
        nthd);
#endif
    ((volatile esKernCtrl_T *)&gKernCtrl)->cthd  = nthd;
    ((volatile esKernCtrl_T *)&gKernCtrl)->pthd  = nthd;
    ((volatile esKernCtrl_T *)&gKernCtrl)->state = ES_KERN_RUN;
    PORT_CRITICAL_EXIT();
}

static void schedRdyAddInitI(
    esThd_T *       thd) {

    if (NULL == gKernCtrl.pthd) {
        ((volatile esKernCtrl_T *)&gKernCtrl)->pthd = thd;
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
                esThd_T * nthd;

                cthd->qCnt = cthd->qRld;                                        /* Reload thread time quantum                               */
                nthd = esThdQRotateI(                                           /* Fetch the next thread and rotate this priority group     */
                    &gRdyQueue,
                    cthd->prio);

                if (cthd == gKernCtrl.pthd) {                                   /* If there is no any other thread pending for switching    */
                    ((volatile esKernCtrl_T *)&gKernCtrl)->pthd = nthd;         /* Make the new thread pending                              */
                }
            }
        }
    }
}

#if (1U == SCHED_POWER_SAVE) || defined(__DOXYGEN__)
static void schedQmActivate(
    void) {
    sysTmrTryActivate();
    gSysTmr.qm |= SYSTMR_SCHED_QM_MSK;
}
#endif

#if (1U == SCHED_POWER_SAVE) || defined(__DOXYGEN__)
static void schedQmDeactivate(
    void) {
    gSysTmr.qm &= ~SYSTMR_SCHED_QM_MSK;
    sysTmrTryDeactivate();
}
#endif

#if (1U == SCHED_POWER_SAVE) || defined(__DOXYGEN__)
static void schedQmEvaluateI(
    esThd_T *       thd) {

    if (DLIST_IS_ENTRY_SINGLE(thdL, thd)) {
        schedQmDeactivate();
    } else {
        schedQmActivate();
    }
}
#endif


/*--  System timer  ----------------------------------------------------------*/

static void sysTmrInit(
    void) {

#if (0U == CFG_SYSTMR_MODE)
    PORT_SYSTMR_INIT();
    PORT_SYSTMR_ENABLE();
    PORT_SYSTMR_ISR_ENABLE();
#elif (1U == CFG_SYSTMR_MODE)
    PORT_SYSTMR_INIT();
    PORT_SYSTMR_ENABLE();
#else
    PORT_SYSTMR_INIT();
#endif
}

#if (0 != CFG_SYSTMR_MODE) || defined(__DOXYGEN__)
static void sysTmrTryActivate(
    void) {

# if (1U == CFG_SYSTMR_MODE)
    if (SYSTMR_INACTIVE == gSysTmr.state) {
        gSysTmr.state = SYSTMR_ACTIVE;
        PORT_SYSTMR_ISR_ENABLE();
    }
# else
    /*
     * TODO: Write adaptive mode
     */
# endif
}
#endif

#if (0 != CFG_SYSTMR_MODE) || defined(__DOXYGEN__)
static void sysTmrTryDeactivate(
    void) {

# if (1U == CFG_SYSTMR_MODE)

    if (SYSTMR_ACTIVE == gSysTmr.state) {
        if (0U == (gSysTmr.tmr + gSysTmr.qm)) {
            gSysTmr.state = SYSTMR_INACTIVE;
            PORT_SYSTMR_ISR_DISABLE();
        }
    }
# else
    /*
     * TODO: Write adaptive mode
     */
# endif
}
#endif


/*--  Timer  -----------------------------------------------------------------*/

static void tmrListAddSort(
    esTmr_T *       list,
    esTmr_T *       tmr) {

    esTmr_T *   tmp;
    esTick_T    tick;

    tmp = DLIST_ENTRY_NEXT(tmrL, list);
    tick = tmr->rtick;

    while (tmp->rtick < tick) {
        tick -= tmp->rtick;
        tmp = DLIST_ENTRY_NEXT(tmrL, tmp);
    }
    tmr->rtick = tick;
    DLIST_ENTRY_ADD_AFTER(tmrL, tmp, tmr);

    if (list != tmp) {
        tmp->rtick -= tmr->rtick;
    }
}


/*--  Kernel threads  --------------------------------------------------------*/

static void kTmrInit(
    void) {

    esThdInit(
        &gKTmrId,
        kTmr,
        NULL,
        gKTmrStck,
        sizeof(gKTmrStck),
        CFG_SCHED_PRIO_LVL - 1U);
    gKTmrId.qCnt = 1U;
    gKTmrId.qRld = 1U;
}

static void kTmr(
    void *          arg) {

    (void)arg;

    while (TRUE) {

        esThdWait();
        ++gSysTmr.tick;

        while (!DLIST_IS_ENTRY_LAST(tmrL, &gTmrPend)) {
            esTmr_T * tmr;

            tmr = DLIST_ENTRY_NEXT(tmrL, &gTmrPend);
            DLIST_ENTRY_RM(tmrL, tmr);
            tmrListAddSort(&gTmrWait, tmr);
        }

        if (!DLIST_IS_ENTRY_LAST(tmrL, &gTmrWait)) {
            esTmr_T * tmr;

            tmr = DLIST_ENTRY_NEXT(tmrL, &gTmrWait);
            --tmr->rtick;

            while (0U == tmr->rtick) {
                DLIST_ENTRY_RM(tmrL, tmr);
                (* tmr->fn)(tmr->arg);
                tmr = DLIST_ENTRY_NEXT(tmrL, &gTmrWait);
            }
        }
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
        PORT_CRITICAL_DECL();

        PORT_CRITICAL_ENTER();
        esKernLockEnterI();
        PORT_CRITICAL_EXIT_SLEEP();
        PORT_CRITICAL_ENTER();
        esKernLockExitI();
        PORT_CRITICAL_EXIT();
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
    kTmrInit();
    PORT_INIT();
}

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

void esSysTmrHandlerI(
    void) {

#if (1U == CFG_HOOK_SYSTMR_EVENT)
    userSysTmr();
#endif

    if (0U != gSysTmr.tmr) {
        esSchedRdyAddI(
            &gKTmrId);
    }
    schedQmI();
}

void esKernLockEnterI(
    void) {

    if (0U == gKernLockCnt) {
        ((volatile esKernCtrl_T *)&gKernCtrl)->state |= SCHED_STATE_LOCK_MSK;
    }
    ++gKernLockCnt;

#if (1U == SCHED_POWER_SAVE)
    schedQmDeactivate();
#endif
}

void esKernLockExitI(
    void) {

    --gKernLockCnt;

    if (0U == gKernLockCnt) {
        ((volatile esKernCtrl_T *)&gKernCtrl)->state &= ~SCHED_STATE_LOCK_MSK;
    }
    esSchedYieldI();
}

void esKernIsrPrologueI(
    void) {

    ES_API_REQUIRE(ES_KERN_INIT > gKernCtrl.state);

    ((volatile esKernCtrl_T *)&gKernCtrl)->state |= SCHED_STATE_INTSRV_MSK;
}

void esKernIsrEpilogueI(
    void) {

    ES_API_REQUIRE(ES_KERN_INIT > gKernCtrl.state);

    if (TRUE == PORT_ISR_IS_LAST()) {
        ((volatile esKernCtrl_T *)&gKernCtrl)->state &= ~SCHED_STATE_INTSRV_MSK;
        esSchedYieldIsrI();
    }
}


/*--  Thread functions  ------------------------------------------------------*/

void esThdInit(
    esThd_T *       thd,
    void (* thdf)(void *),
    void *          arg,
    portStck_T *    stck,
    size_t          stckSize,
    uint8_t         prio) {

	PORT_CRITICAL_DECL();

    ES_API_REQUIRE(ES_KERN_INACTIVE > gKernCtrl.state);
    ES_API_REQUIRE(NULL != thd);
    ES_API_REQUIRE(NULL != thdf);
    ES_API_REQUIRE(NULL != stck);
    ES_API_REQUIRE(PORT_STCK_MINSIZE_VAL <= (stckSize * sizeof(portReg_T)));
    ES_API_REQUIRE(CFG_SCHED_PRIO_LVL >= prio);

    thd->stck   = PORT_CTX_INIT(stck, stckSize, thdf, arg);                     /* Make a fake stack                                        */
    thd->thdL.q = NULL;                                                         /* This thread is not in any thread queue                   */
    DLIST_ENTRY_INIT(thdL, thd);
    thd->prio   = prio;                                                         /* Set the priority                                         */
    thd->cprio  = prio;                                                         /* This is constant priority, it never changes              */
    thd->qCnt   = CFG_SCHED_TIME_QUANTUM;
    thd->qRld   = CFG_SCHED_TIME_QUANTUM;
    ES_API_OBLIGATION(thd->signature = THD_CONTRACT_SIGNATURE);                 /* Make thread structure valid                              */
    PORT_CRITICAL_ENTER();
    schedRdyAddInitI(
        thd);
    esSchedRdyAddI(
        thd);
    esSchedYieldI();
    PORT_CRITICAL_EXIT();

#if (1U == CFG_HOOK_THD_INIT_END)
    userThdInitEnd();
#endif
}

void esThdTerm(
    esThd_T *       thd) {

    PORT_CRITICAL_DECL();

    ES_API_REQUIRE(ES_KERN_INACTIVE > gKernCtrl.state);
    ES_API_REQUIRE(NULL != thd);
    ES_API_REQUIRE(THD_CONTRACT_SIGNATURE == thd->signature);
    ES_API_REQUIRE((NULL == thd->thdL.q) || (&gRdyQueue == thd->thdL.q));

#if (1U == CFG_HOOK_THD_TERM)
    userThdTerm();
#endif
    PORT_CRITICAL_ENTER();

    if (&gRdyQueue == thd->thdL.q) {
        esSchedRdyRmI(
            thd);
    }
    ES_API_OBLIGATION(thd->signature = 0U);
    esSchedYieldI();
    PORT_CRITICAL_EXIT();
}

void esThdSetPrioI(
    esThd_T *       thd,
    uint8_t         prio) {

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
                ((volatile esKernCtrl_T *)&gKernCtrl)->pthd = thd;              /* Notify scheduler about new thread                        */
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
            ((volatile esKernCtrl_T *)&gKernCtrl)->pthd = esThdQFetchFirstI(&gRdyQueue);
        }
    }
}

void esThdPostI(
    esThd_T *       thd) {

    if (NULL == thd->thdL.q) {
        esSchedRdyAddI(
            thd);
    }
    esSchedYieldI();
}

void esThdPost(
    esThd_T *       thd) {

    PORT_CRITICAL_DECL();

    PORT_CRITICAL_ENTER();
    esThdPostI(
        thd);
    PORT_CRITICAL_EXIT();
}

void esThdWaitI(
    void) {

    esSchedRdyRmI(
        esThdGetId());
    esSchedYieldI();
}

void esThdWait(
    void) {

    PORT_CRITICAL_DECL();

    PORT_CRITICAL_ENTER();
    esThdWaitI();
    PORT_CRITICAL_EXIT();
}


/*--  Thread Queue functions  ------------------------------------------------*/

void esThdQInit(
    esThdQ_T *      thdQ) {

    uint8_t         group;

    ES_API_REQUIRE(NULL != thdQ);

    prioBMInit(
        &thdQ->prioOcc);

    for (group = 0U; group < CFG_SCHED_PRIO_LVL; group++) {
        thdQ->grp[group] = NULL;
    }
    ES_API_OBLIGATION(thdQ->signature = THDQ_CONTRACT_SIGNATURE);
}

void esThdQAddI(
    esThdQ_T *      thdQ,
    esThd_T *       thd) {

    esThd_T **      sentinel;

    ES_API_REQUIRE(NULL != thdQ);
    ES_API_REQUIRE(THDQ_CONTRACT_SIGNATURE == thdQ->signature);
    ES_API_REQUIRE(NULL != thd);
    ES_API_REQUIRE(THD_CONTRACT_SIGNATURE == thd->signature);
    ES_API_REQUIRE(NULL == thd->thdL.q);
    ES_API_REQUIRE(CFG_SCHED_PRIO_LVL >= thd->prio);

    sentinel = &(thdQ->grp[thd->prio]);

    if (NULL == *sentinel) {                                                    /* Is thdL list empty?                                      */
        *sentinel = thd;                                                        /* This thread becomes first in the list                    */
        prioBMSet(
            &thdQ->prioOcc,
            thd->prio);                                                         /* Mark the priority group as used.                         */
    } else {                                                                    /* No, thdL list is occupied.                               */
        DLIST_ENTRY_ADD_AFTER(thdL, *sentinel, thd);
    }
    thd->thdL.q = thdQ;
}

void esThdQRmI(
    esThdQ_T *      thdQ,
    esThd_T *       thd) {

    esThd_T **      sentinel;

    ES_API_REQUIRE(NULL != thd);
    ES_API_REQUIRE(THD_CONTRACT_SIGNATURE == thd->signature);
    ES_API_REQUIRE(NULL != thdQ);
    ES_API_REQUIRE(THDQ_CONTRACT_SIGNATURE == thdQ->signature);
    ES_API_REQUIRE(thdQ == thd->thdL.q);
    ES_API_REQUIRE(CFG_SCHED_PRIO_LVL >= thd->prio);

    sentinel = &(thdQ->grp[thd->prio]);

    if (DLIST_IS_ENTRY_FIRST(thdL, thd)) {                                      /* Is this thread last one in the thdL list?                */
        *sentinel = NULL;                                                       /* Make the list empty.                                     */
        prioBMClear(
            &thdQ->prioOcc,
            thd->prio);                                                         /* Remove the mark since this group is not used.            */
    } else {                                                                    /* This thread is not the last one in the thdL list.        */

        if (*sentinel == thd) {                                                 /* In case we are removing thread from the beginning of the */
            *sentinel = DLIST_ENTRY_NEXT(thdL, thd);                            /* list we need to advance sentinel to point to the next one*/
        }                                                                       /* in list.                                                 */
        DLIST_ENTRY_RM(thdL, thd);
        DLIST_ENTRY_INIT(thdL, thd);
    }
    thd->thdL.q = NULL;
}

esThd_T * esThdQFetchFirstI(
    const esThdQ_T *    thdQ) {

    esThd_T **      sentinel;
    uint_fast8_t    prio;

    ES_API_REQUIRE(NULL != thdQ);
    ES_API_REQUIRE(THDQ_CONTRACT_SIGNATURE == thdQ->signature);

    prio = prioBMGet(
        &thdQ->prioOcc);                                                        /* Get the highest priority ready to run.                   */
    sentinel = (esThd_T **)&(thdQ->grp[prio]);                                  /* Get the Group Head pointer for that priority.            */

    return (*sentinel);
}

esThd_T * esThdQRotateI(
    esThdQ_T *      thdQ,
    uint_fast8_t    prio) {

    esThd_T **      sentinel;

    ES_API_REQUIRE(NULL != thdQ);
    ES_API_REQUIRE(THDQ_CONTRACT_SIGNATURE == thdQ->signature);
    ES_API_REQUIRE(CFG_SCHED_PRIO_LVL >= prio);

    sentinel = &(thdQ->grp[prio]);                                              /* Get the Group Head pointer from thread priority.         */
    *sentinel = DLIST_ENTRY_NEXT(thdL, *sentinel);

    return (*sentinel);
}

bool_T esThdQIsEmpty(
    const esThdQ_T *    thdQ) {

    bool_T ans;

    ES_API_REQUIRE(NULL != thdQ);
    ES_API_REQUIRE(THDQ_CONTRACT_SIGNATURE == thdQ->signature);

    ans = prioBMIsEmpty(&thdQ->prioOcc);

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
        ((volatile esKernCtrl_T *)&gKernCtrl)->pthd = thd;
    }
}

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
        ((volatile esKernCtrl_T *)&gKernCtrl)->pthd = esThdQFetchFirstI(&gRdyQueue);
    }
}

void esSchedYieldI(
    void) {

    ES_API_REQUIRE(ES_KERN_INACTIVE > gKernCtrl.state);

    if (ES_KERN_RUN == gKernCtrl.state) {
        esThd_T * newThd;

        newThd = gKernCtrl.pthd;

#if (1U == SCHED_POWER_SAVE)
        schedQmEvaluateI(
            newThd);
#endif

        if (newThd != gKernCtrl.cthd) {

#if (1U == CFG_HOOK_CTX_SW)
            userCtxSw(
                gKernCtrl.cthd,
                newThd);
#endif
            PORT_CTX_SW();
        }
    }
}

void esSchedYieldIsrI(
    void) {

    ES_API_REQUIRE(ES_KERN_INACTIVE > gKernCtrl.state);

    if (ES_KERN_RUN == gKernCtrl.state) {
        esThd_T * newThd;

        newThd = gKernCtrl.pthd;

#if (1U == SCHED_POWER_SAVE)
        schedQmEvaluateI(
            newThd);
#endif

        if (newThd != gKernCtrl.cthd) {

#if (1U == CFG_HOOK_CTX_SW)
        userCtxSw(
            gKernCtrl.cthd,
            newThd);
#endif
            PORT_CTX_SW_ISR();
        }
    }
}


/*--  System timer functions  ------------------------------------------------*/

void esSysTmrEnable(
    void) {

#if (0U != CFG_SYSTMR_MODE)
    PORT_CRITICAL_DECL();

    PORT_CRITICAL_ENTER();
    sysTmrTryActivate();
    gSysTmr.qm |= SYSTMR_USR_QM_MSK;
    PORT_CRITICAL_EXIT();
#endif
}

void esSysTmrDisable(
    void) {

#if (0U != CFG_SYSTMR_MODE)
    PORT_CRITICAL_DECL();

    PORT_CRITICAL_ENTER();
    gSysTmr.qm &= ~SYSTMR_USR_QM_MSK;
    sysTmrTryDeactivate();
    PORT_CRITICAL_EXIT();
#endif
}


/*--  Timer functions  -------------------------------------------------------*/

void esTmrAddI(
    esTmr_T *       tmr,
    esTick_T        tick,
    void (* fn)(void *),
    void *          arg) {

    tmr->rtick = tick;
    tmr->fn = fn;
    tmr->arg = arg;
    DLIST_ENTRY_ADD_AFTER(tmrL, &gTmrPend, tmr);

#if (0U != CFG_SYSTMR_MODE)
    sysTmrTryActivate();
#endif
    ++gSysTmr.tmr;
}

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of kernel.c
 ******************************************************************************/
