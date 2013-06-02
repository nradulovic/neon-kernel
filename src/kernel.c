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

/**@brief       Priority Bit Map log base 2: <code>log2(PORT_DATA_WIDTH)</code>
 */
#define PRIO_BM_DATA_WIDTH_LOG2                                                 \
    (PORT_DATA_WIDTH <   2 ? 0 :                                                \
     (PORT_DATA_WIDTH <   4 ? 1 :                                               \
      (PORT_DATA_WIDTH <   8 ? 2 :                                              \
       (PORT_DATA_WIDTH <  16 ? 3 :                                             \
        (PORT_DATA_WIDTH <  32 ? 4 :                                            \
         (PORT_DATA_WIDTH <  64 ? 5 :                                           \
          (PORT_DATA_WIDTH < 128 ? 6 : 7)))))))

/**@brief       Kernel state variable bit position which defines if kernel is in
 *              interrupt servicing state
 */
#define SCHED_STATE_ISR_ACTIVE_MSK              (1U << 0U)

/**@brief       Kernel state variable bit position which defines if the kernel
 *              is locked or not.
 */
#define SCHED_STATE_LOCK_MASK                   (1U << 1U)

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

/**@brief       Helper macro: is the thread the only one in the list
 */
#define THDQ_IS_THD_FIRST(thd)          ((thd) == (thd)->thdL.next)

/**@brief       Helper macro: is the thread second one in the list
 */
#define THDQ_IS_THD_SECOND(thd)                                                 \
    (((thd)->thdL.next == (thd)->thdL.prev) && ((thd) != (thd)->thdL.next))

/**@brief       System Timer kernel thread stack size
 */
#define KSYSTMR_STCK_SIZE              PORT_STCK_SIZE(40U)

/**@brief       Idle kernel thread stack size
 */
#define KIDLE_STCK_SIZE                PORT_STCK_SIZE(40U)

/*======================================================  LOCAL DATA TYPES  ==*/

/*------------------------------------------------------------------------*//**
 * @name        System Timer
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       System Timer state enumeration
 */
enum sysTmrState {
    SYSTMR_ENABLE   = 0x00U,                                                    /**< System Timer is enabled.                               *///!< SYSTMR_ENABLE
    SYSTMR_DISABLE  = 0x01U                                                     /**< System Timer is disabled.                              *///!< SYSTMR_DISABLE
};

/**@brief       System Timer structure
 */
struct sysTmr {
    uint_fast16_t       cnt;                                                    /**< @brief Number of system timer users.                   */
    enum sysTmrState    state;                                                  /**< @brief System Timer state                              */
    struct esTmr *      head;
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
 *              Kernel control structure @ref esKernCntl.
 */
static PORT_C_INLINE void schedInit(
    void);

/**@brief       Set scheduler data structures ready for multi-threading
 * @details     This function is called just before multi-threading has
 *              commenced.
 */
static PORT_C_INLINE void schedStartI(
    void);

/**@brief       Do Round-Robin scheduling
 */
static void schedSysTmrI(
    void);

/**@} *//*----------------------------------------------------------------*//**
 * @name        System timer
 * @{ *//*--------------------------------------------------------------------*/

static void sysTmrTryStartI(
    portReg_T       state);

static portReg_T sysTmrTryStopI(
    void);

/**@} *//*----------------------------------------------------------------*//**
 * @name        System timer kernel thread
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Initialization of System Timer Thread
 */
static void kSysTmrInit(
    void);

/**@brief       System timer thread code
 * @param       arg
 *              NO ARGUMENTS - thread does not use argument
 */
static void kSysTmr(
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

/**@brief       Kernel Lock Counter
 */
static uint_fast8_t gLockCnt = 0U;

/**@} *//*----------------------------------------------------------------*//**
 * @name        System timer kernel thread
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       System Timer
 */
static sysTmr_T gSysTmr = {
    0U,
    SYSTMR_DISABLE,
    NULL
};

/**@brief       System timer thread Id
 */
static esThd_T kSysTmrId;

/**@brief       System timer thread stack
 */
static portStck_T gKSysTmrStck[KSYSTMR_STCK_SIZE];

/**@} *//*----------------------------------------------------------------*//**
 * @name        Idle kernel thread
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Idle thread Id
 */
static esThd_T kIdleId;

/**@brief       Idle thread stack
 */
static portStck_T gKIdleStck[KIDLE_STCK_SIZE];

/**@} *//*--------------------------------------------------------------------*/
/*======================================================  GLOBAL VARIABLES  ==*/

/**@brief       Kernel control initialization
 */
const volatile esKernCntl_T gKernCntl = {
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
    ((volatile esKernCntl_T *)&gKernCntl)->cthd  = NULL;
    ((volatile esKernCntl_T *)&gKernCntl)->pthd  = NULL;
    ((volatile esKernCntl_T *)&gKernCntl)->state = ES_KERN_INIT;
}

static PORT_C_INLINE void schedStartI(
    void) {

    esThd_T * nthd;

    esSysTmrEvaluateI();                                                        /* Check if system timer is needed for Round-Robin          */
    nthd = esThdQFetchFirstI(                                                   /* Get the highest priority thread                          */
            &gRdyQueue);
    ((volatile esKernCntl_T *)&gKernCntl)->cthd  = nthd;
    ((volatile esKernCntl_T *)&gKernCntl)->pthd  = nthd;
    ((volatile esKernCntl_T *)&gKernCntl)->state = ES_KERN_RUN;
}

static void schedSysTmrI(
    void) {

    if (ES_KERN_LOCK > gKernCntl.state) {                                       /* Round-Robin is not enabled in kernel LOCK state          */

        esThd_T * cthd;

        cthd = gKernCntl.cthd;                                                  /* Get the current thread                                   */

        if (!THDQ_IS_THD_FIRST(cthd)) {
            cthd->qCnt--;                                                       /* Decrement current thread time quantum                    */

            if (0U == cthd->qCnt) {
                esThd_T * nthd;

                cthd->qCnt = cthd->qRld;                                        /* Reload thread time quantum                               */
                nthd = esThdQRotateI(                                           /* Fetch the next thread and rotate this priority group      */
                    &gRdyQueue,
                    cthd->prio);

                if (cthd == gKernCntl.pthd) {                                   /* If there is no any other thread pending for switching    */
                    ((volatile esKernCntl_T *)&gKernCntl)->pthd = nthd;         /* Make the new thread pending                              */
                }
            }
        }
    }
}


/*--  System timer  ----------------------------------------------------------*/

static portReg_T sysTmrTryStopI(
    void) {

    portReg_T ans;

    if (0U == gSysTmr.cnt) {
        gSysTmr.state = SYSTMR_DISABLE;
        PORT_SYSTMR_TERM();
        ans = 0U;
    } else {
        PORT_SYSTMR_RELOAD(1U);
        ans = 1U;
    }

    return (ans);
}

static void sysTmrTryStartI(
    portReg_T       state) {

    if (0U == state) {
        PORT_SYSTMR_INIT();
    } else {
        PORT_SYSTMR_RELOAD(1U);
    }
}


/*--  Kernel threads  --------------------------------------------------------*/

static void kSysTmrInit(
    void) {

    esThdInit(
        &kSysTmrId,
        kSysTmr,
        NULL,
        gKSysTmrStck,
        sizeof(gKSysTmrStck),
        CFG_SCHED_PRIO_LVL - 1U);
    kSysTmrId.qCnt = 1U;
    kSysTmrId.qRld = 1U;
}

static void kSysTmr(
    void *          arg) {

    esTick_T      sysTmrCnt;

    (void)arg;
    PORT_SYSTMR_INIT();
    gSysTmr.state = SYSTMR_DISABLE;
    sysTmrCnt = 0U;

    while (TRUE) {
        esThdWait();
        ++sysTmrCnt;
    }
}

static void kIdleInit(
    void) {

    esThdInit(
        &kIdleId,
        kIdle,
        NULL,
        gKIdleStck,
        sizeof(gKIdleStck),
        0U);
    kIdleId.qCnt = 1U;
    kIdleId.qRld = 1U;
}

static void kIdle(
    void *          arg){

    (void)arg;

    while (TRUE) {
        PORT_CRITICAL_DECL();
        portReg_T   tmrState;

        PORT_CRITICAL_ENTER();
        esKernLockEnterI();
        tmrState = sysTmrTryStopI();
        PORT_CRITICAL_EXIT_SLEEP(tmrState);
        PORT_CRITICAL_ENTER();
        sysTmrTryStartI(tmrState);
        esKernLockExitI();
        PORT_CRITICAL_EXIT();
    }
}

/*===================================  GLOBAL PRIVATE FUNCTION DEFINITIONS  ==*/
/*====================================  GLOBAL PUBLIC FUNCTION DEFINITIONS  ==*/


/*--  Kernel General functions  ----------------------------------------------*/

void esKernInit(
    void) {

    ES_API_REQUIRE(ES_KERN_INACTIVE == gKernCntl.state);

#if (1U == CFG_HOOK_KERN_INIT)
    userKernInit();
#endif
    PORT_INT_DISABLE();
    PORT_INIT_EARLY();
    schedInit();
    kIdleInit();
    kSysTmrInit();
    PORT_INIT();
}

PORT_C_NORETURN void esKernStart(
    void) {

    PORT_CRITICAL_DECL();

    ES_API_REQUIRE(ES_KERN_INIT == gKernCntl.state);
    ES_API_REQUIRE(FALSE == esThdQIsEmpty(&gRdyQueue));

#if (1U == CFG_HOOK_KERN_START)
    userKernStart();
#endif
    PORT_INIT_LATE();
    PORT_CRITICAL_ENTER();
    schedStartI();                                                              /* Initialize scheduler data structures for multi-threading */
    PORT_CRITICAL_EXIT();
    PORT_THD_START();                                                           /* Start the first thread                                   */

    while (TRUE) {
        ;
    }
}

void esSysTmrHandlerI(void) {

    ES_API_REQUIRE(ES_KERN_INIT > gKernCntl.state);

#if (1U == CFG_HOOK_SYSTMR_EVENT)
    userSysTmr();
#endif
    esSchedRdyAddI(
        &kSysTmrId);
    schedSysTmrI();
}

void esKernLockEnterI(
    void) {

    if (0U == gLockCnt) {
        ((volatile esKernCntl_T *)&gKernCntl)->state |= SCHED_STATE_LOCK_MASK;
    }
    ++gLockCnt;
}

void esKernLockExitI(
    void) {

    --gLockCnt;

    if (0U == gLockCnt) {
        ((volatile esKernCntl_T *)&gKernCntl)->state &= ~SCHED_STATE_LOCK_MASK;
    }
}

void esKernIsrPrologueI(
    void) {

    ES_API_REQUIRE(ES_KERN_INIT > gKernCntl.state);

    ((volatile esKernCntl_T *)&gKernCntl)->state |= SCHED_STATE_ISR_ACTIVE_MSK;
}

void esKernIsrEpilogueI(
    void) {

    ES_API_REQUIRE(ES_KERN_INIT > gKernCntl.state);

    if (TRUE == PORT_ISR_IS_LAST()) {
        ((volatile esKernCntl_T *)&gKernCntl)->state &= ~SCHED_STATE_ISR_ACTIVE_MSK;

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

    ES_API_REQUIRE(ES_KERN_INACTIVE > gKernCntl.state);
	ES_API_REQUIRE(NULL != thd);
	ES_API_REQUIRE(NULL != thdf);
	ES_API_REQUIRE(NULL != stck);
	ES_API_REQUIRE(PORT_STCK_MINSIZE <= (stckSize * sizeof(portReg_T)));
	ES_API_REQUIRE(CFG_SCHED_PRIO_LVL >= prio);

    thd->stck   = PORT_CTX_INIT(stck, stckSize, thdf, arg);                     /* Make a fake stack                                        */
    thd->thdL.q = NULL;                                                         /* This thread is not in any thread queue                   */
    thd->prio   = prio;                                                         /* Set the priority                                         */
    thd->cprio  = prio;                                                         /* This is constant priority, it never changes              */
    thd->qCnt   = CFG_SCHED_TIME_QUANTUM;
    thd->qRld   = CFG_SCHED_TIME_QUANTUM;
    ES_API_OBLIGATION(thd->signature = THD_CONTRACT_SIGNATURE);                 /* Make thread structure valid                              */
    PORT_CRITICAL_ENTER();
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

    ES_API_REQUIRE(ES_KERN_INACTIVE > gKernCntl.state);
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

    ES_API_REQUIRE(ES_KERN_INACTIVE > gKernCntl.state);
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

            if (thd->prio > gKernCntl.pthd->prio) {                             /* If new prio is higher than the current prio              */
                ((volatile esKernCntl_T *)&gKernCntl)->pthd = thd;              /* Notify scheduler about new thread                        */
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
            ((volatile esKernCntl_T *)&gKernCntl)->pthd = NULL;
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

    if (NULL == *sentinel) {                                                    /* Is thdL list empty?                                       */
        *sentinel = thd;                                                        /* This thread becomes first in the list                    */
        thd->thdL.next = thd;
        thd->thdL.prev = thd;
        prioBMSet(
            &thdQ->prioOcc,
            thd->prio);                                                         /* Mark the priority group as used.                         */
    } else {                                                                    /* No, thdL list is occupied.                                */
        thd->thdL.next = *sentinel;
        thd->thdL.prev = (*sentinel)->thdL.prev;
        thd->thdL.prev->thdL.next = thd;
        thd->thdL.next->thdL.prev = thd;
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

    if (THDQ_IS_THD_FIRST(thd)) {                                               /* Is this thread last one in the thdL list?                */
        *sentinel = NULL;                                                       /* Make the list empty.                                     */
        prioBMClear(
            &thdQ->prioOcc,
            thd->prio);                                                         /* Remove the mark since this group is not used.            */
    } else {                                                                    /* This thread is not the last one in the thdL list.        */

        if (*sentinel == thd) {                                                 /* In case we are removing thread from the beginning of the */
            *sentinel = thd->thdL.next;                                         /* list we need to advance sentinel to point to the next one*/
        }                                                                       /* in list.                                                 */
        thd->thdL.next->thdL.prev = thd->thdL.prev;
        thd->thdL.prev->thdL.next = thd->thdL.next;
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
    *sentinel = (*sentinel)->thdL.next;

    return (*sentinel);
}

bool_T esThdQIsEmpty(
    const esThdQ_T *    thdQ) {

    bool_T ans;

    ES_API_REQUIRE(NULL != thdQ);
    ES_API_REQUIRE(THDQ_CONTRACT_SIGNATURE == thdQ->signature);

    if (TRUE == prioBMIsEmpty(&thdQ->prioOcc)) {
        ans = TRUE;
    } else {
        ans = FALSE;
    }

    return (ans);
}


/*--  Scheduler functions  ---------------------------------------------------*/

void esSchedRdyAddI(
    esThd_T *       thd) {

    esThd_T *       nthd;

    ES_API_REQUIRE(ES_KERN_INACTIVE > gKernCntl.state);
    ES_API_REQUIRE(NULL != thd);
    ES_API_REQUIRE(THD_CONTRACT_SIGNATURE == thd->signature);
    ES_API_REQUIRE(NULL == thd->thdL.q);

    esThdQAddI(
        &gRdyQueue,
        thd);

    if (THDQ_IS_THD_SECOND(thd)) {
        sysTmrTAddI();
    }
    nthd = gKernCntl.pthd;

    if (NULL != nthd) {

        if (thd->prio > nthd->prio) {
            ((volatile esKernCntl_T *)&gKernCntl)->pthd = thd;
        }
    }
}

void esSchedRdyRmI(
    esThd_T *       thd) {

    ES_API_REQUIRE(ES_KERN_INACTIVE > gKernCntl.state);
    ES_API_REQUIRE(NULL != thd);
    ES_API_REQUIRE(THD_CONTRACT_SIGNATURE == thd->signature);
    ES_API_REQUIRE(&gRdyQueue == thd->thdL.q);

    if (THDQ_IS_THD_SECOND(thd)) {
        sysTmrTRmI();
    }
    esThdQRmI(
        &gRdyQueue,
        thd);

    if ((gKernCntl.cthd == thd) || (gKernCntl.pthd == thd)) {
        ((volatile esKernCntl_T *)&gKernCntl)->pthd = NULL;
    }
}

void esSchedYieldI(
    void) {

    ES_API_REQUIRE(ES_KERN_INACTIVE > gKernCntl.state);

    if (ES_KERN_RUN == gKernCntl.state) {
        esThd_T * newThd;

        newThd = gKernCntl.pthd;

        if (NULL == newThd) {
            newThd = esThdQFetchFirstI(
                &gRdyQueue);
            ((volatile esKernCntl_T *)&gKernCntl)->pthd = newThd;
        }

        if (newThd != gKernCntl.cthd) {
            esSysTmrEvaluateI();

#if (1U == CFG_HOOK_CTX_SW)
            userCtxSw(
                gKernCntl.cthd,
                newThd);
#endif
            PORT_CTX_SW();
        }
    }
}

void esSchedYieldIsrI(
    void) {

    ES_API_REQUIRE(ES_KERN_INACTIVE > gKernCntl.state);

    if (ES_KERN_RUN == gKernCntl.state) {
        esThd_T * newThd;

        newThd = gKernCntl.pthd;

        if (NULL == newThd) {
            newThd = esThdQFetchFirstI(
                &gRdyQueue);
            ((volatile esKernCntl_T *)&gKernCntl)->pthd = newThd;
        }

        if (newThd != gKernCntl.cthd) {
            esSysTmrEvaluateI();

#if (1U == CFG_HOOK_CTX_SW)
        userCtxSw(
            gKernCntl.cthd,
            newThd);
#endif
            PORT_CTX_SW_ISR();
        }
    }
}


/*--  System timer functions  ------------------------------------------------*/

void esSysTmrEvaluateI(
    void) {

    switch (gSysTmr.state) {
        case SYSTMR_DISABLE : {

            if (0U < gSysTmr.cnt) {
                gSysTmr.state = SYSTMR_ENABLE;
                PORT_SYSTMR_ISR_ENABLE();
            }
            break;
        }

        case SYSTMR_ENABLE : {

            if (0U == gSysTmr.cnt) {
                gSysTmr.state = SYSTMR_DISABLE;
                PORT_SYSTMR_ISR_DISABLE();
            }
            break;
        }
    }
}

void sysTmrTAddI(
    void) {

    ++gSysTmr.cnt;
}

void sysTmrTRmI(
    void) {

    --gSysTmr.cnt;
}

void esTmrAddI(
    esTmr_T *       tmr,
    esTick_T        tick,
    void (* fn)(void *),
    void *          arg) {

    tmr->fn = fn;
    tmr->arg = arg;

    if (NULL != gSysTmr.head) {
        tmr->next = tmr;
        tmr->prev = tmr;
        tmr->rtick = tick;
    } else {
        esTmr_T *   tmp;

        tmp = gSysTmr.head;

        while (tmp->rtick < tick) {
            tick -= tmp->rtick;
            tmp = tmp->next;
        }
        tmr->rtick = tick;
        tmr->next = tmp;
        tmr->prev = tmp->prev;
        tmr->prev->next = tmr;
        tmr->next->prev = tmr;

        if (tmp != gSysTmr.head) {
            tmp->rtick -= tick;
        }
    }
}

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of kernel.c
 ******************************************************************************/
