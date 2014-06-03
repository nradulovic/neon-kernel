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

#include "kernel/nsys.h"

/*=========================================================  LOCAL MACRO's  ==*/

/**@brief       Kernel state variable bit position which defines if the kernel
 *              is in interrupt servicing state.
 */
#define DEF_SCHED_STATE_INTSRV_MSK      (0x01u << 0)

/**@brief       Kernel state variable bit position which defines if the kernel
 *              is locked or not.
 */
#define DEF_SCHED_STATE_LOCK_Msk        (0x01u << 1)

/**@brief       Thread structure signature.
 * @details     The signature is used to confirm that a structure passed to a
 *              kernel function is indeed a esThread thread structure.
 */
#define DEF_THD_CONTRACT_SIGNATURE      ((natomic)0xfeedbeeful)

/**@brief       Thread Queue structure signature.
 * @details     The signature is used to confirm that a structure passed to a
 *              kernel function is indeed a esThreadQ thread queue structure.
 */
#define DEF_THDQ_CONTRACT_SIGNATURE     ((natomic)0xfeedbef0ul)

/**@brief       Timer structure signature.
 * @details     The signature is used to confirm that a structure passed to a
 *              timer function is indeed a esVTmr_T timer structure.
 */
#define DEF_VTMR_CONTRACT_SIGNATURE     ((natomic)0xfeedbef1ul)

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
    nthread *           thd);

/**@brief       Wait for a signal
 */
void thdWait(
    void);

/**@} *//*--------------------------------------------------------------------*/

/*=======================================================  LOCAL VARIABLES  ==*/

/**@brief       Module identification info
 */
static const NMODULE_INFO_CREATE("Kernel", ES_KERN_ID, "Nenad Radulovic");

/**@} *//*----------------------------------------------------------------*//**
 * @name        System timer
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Main System Timer structure
 */
static struct sysTmr SysTmr =
{
    0u,
    0u,
    0u,
#if   (1u == CFG_SYSTMR_ADAPTIVE_MODE)
    0u
#endif
};

/**@brief       List of virtual armed timers waiting to expire
 */
static struct esVTmr VTmrArmed =
{
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
#if   (1u == CONFIG_API_VALIDATION)
   DEF_VTMR_CONTRACT_SIGNATURE
#endif
};

/**@brief       Virtual timers pending to be inserted into waiting list
 */
static struct esVTmr VTmrPend =
{
   {
        &VTmrPend,
        &VTmrPend,
        &VTmrPend
   },
   0u,
   NULL,
   NULL,
#if   (1u == CONFIG_API_VALIDATION)
   DEF_VTMR_CONTRACT_SIGNATURE
#endif
};

/**@brief       Virtual timer thread ID
 */
static struct nthread KVTmr;

/**@} *//*----------------------------------------------------------------*//**
 * @name        Idle kernel thread
 * @{ *//*--------------------------------------------------------------------*/

/**@brief       Idle thread ID
 */
static struct nthread KIdle;

/**@} *//*--------------------------------------------------------------------*/
/*======================================================  GLOBAL VARIABLES  ==*/
/*============================================  LOCAL FUNCTION DEFINITIONS  ==*/

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

        vTmr = NDLIST_NEXT(tmrL, &VTmrArmed);

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
            nsched_add_thread_i(
                &KVTmr);
        }
    }

    if (0u != SysTmr.vTmrPend) {                                                /* There is a timer pending, start kVTmr thread.            */

        if (nthread_queue_get_container(&KVTmr) == NULL) {
            nsched_add_thread_i(&KVTmr);
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
    tmp = NDLIST_NEXT(tmrL, &VTmrArmed);
    tick = vTmr->rtick;

    while (tmp->rtick <= tick) {
        tick -= tmp->rtick;
        tmp = NDLIST_NEXT(tmrL, tmp);
    }
    vTmr->rtick = tick;
    NDLIST_ADD_AFTER(tmrL, tmp, vTmr);

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

    nintr_ctx           intr_ctx;

    nsched_lock_int_enter(&intr_ctx);

    while (0u != SysTmr.vTmrPend) {
        esVTmr_T * tmr;

        --SysTmr.vTmrPend;
        ++SysTmr.vTmrArmed;
        tmr = NDLIST_NEXT(tmrL, &VTmrPend);
        NDLIST_REMOVE(tmrL, tmr);
        NCRITICAL_LOCK_EXIT(intr_ctx);
        --tmr->rtick;                                                           /* Timer thread requires one tick less than original value. */
        vTmrAddArmedS(
            tmr);
        NCRITICAL_LOCK_ENTER(&intr_ctx);
    }
    nsched_lock_int_exit(intr_ctx);
}

/* 1)       Kernel Virtual Timer thread must have the highest priority available.
 */
static void kVTmrInit(
    void) {

    static nthread_stack kVTmrStck[NSTACK_SIZE(PORT_DEF_KVTMR_STCK_SIZE)];        /* Virtual timer kernel thread stack.                       */

    nthread_init(
        &KVTmr,
        kVTmr,
        NULL,
        kVTmrStck,
        sizeof(kVTmrStck),
        CONFIG_PRIORITY_LEVELS - 1u);
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

            tmr = NDLIST_PREV(tmrL, &VTmrArmed);

            while (0u == tmr->rtick) {
                esVTmr_T * tmpTmr;

                --SysTmr.vTmrArmed;
                NDLIST_REMOVE(tmrL, tmr);
                tmpTmr = tmr;
                NOBLIGATION(tmr->signature = ~DEF_VTMR_CONTRACT_SIGNATURE);
                tmr = NDLIST_NEXT(tmrL, &VTmrArmed);
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

    static nthread_stack kIdleStck[NSTACK_SIZE(PORT_DEF_KIDLE_STCK_SIZE)];        /* Idle kernel thread stack.                                */

    nthread_init(
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
        sched_sleep();
#endif
    }
}


/*--  Basic thread synchronization  ------------------------------------------*/

/* 1)       Since this function can be called multiple times with the same
 *          thread then it needs to check if the thread is not already added in
 *          a queue.
 */
void thdPost(
    nthread *           thd) {

    nintr_ctx           intrCtx;

    NCRITICAL_LOCK_ENTER(&intrCtx);
    if (nthread_queue_get_container(thd) == NULL)
    {
        nsched_add_thread_i(thd);
        nsched_yield_i();
    }
    NCRITICAL_LOCK_EXIT(intrCtx);
}

void thdWait(
    void) {

    nintr_ctx           intrCtx;

    NCRITICAL_LOCK_ENTER(&intrCtx);
    nsched_remove_thread_i(nsched_get_current());
    nsched_yield_i();
    NCRITICAL_LOCK_EXIT(intrCtx);
}

/*===================================  GLOBAL PRIVATE FUNCTION DEFINITIONS  ==*/
/*====================================  GLOBAL PUBLIC FUNCTION DEFINITIONS  ==*/


/*--  General kernel functions  ----------------------------------------------*/

void nsys_init(
    void) {

    NREQUIRE(NAPI_USAGE, NSCHED_INACTIVE == global_sched_ctx.state);

#if   (1u == CFG_HOOK_PRE_KERN_INIT)
    userPreKernInit();
#endif
    ES_INTR_DISABLE();
    PORT_KCORE_INIT_EARLY();
    sysTmrInit();
    nsched_init();
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
PORT_C_NORETURN void nsys_start(
    void) {

    NREQUIRE(NAPI_USAGE, NSCHED_INIT == global_sched_ctx.state);

#if   (1u == CFG_HOOK_PRE_KERN_START)
    userPreKernStart();
#endif
    PORT_KCORE_INIT_LATE();
    nsched_start();                                                               /* Initialize scheduler data structures for multi-threading */

    while (true);                                                               /* Prevent compiler `function does return` warnings.        */
}

void esKernSysTmr(
    void) {

    nintr_ctx           intrCtx;

    NREQUIRE(NAPI_USAGE, global_sched_ctx.cthread != &KVTmr);

#if   (1u == CFG_HOOK_PRE_SYSTMR_EVENT)
    userPreSysTmr();
#endif
    NCRITICAL_LOCK_ENTER(&intrCtx);
    vTmrEvaluateI();
    sched_quantum_i();
    NCRITICAL_LOCK_EXIT(intrCtx);
}

void nsys_isr_enter_i(
    void) {

    NREQUIRE(NAPI_USAGE, NSCHED_INIT > global_sched_ctx.state);

    PORT_ISR_ENTER();
    ((struct nsched_ctx *)&global_sched_ctx)->state |= DEF_SCHED_STATE_INTSRV_MSK;
}

void nsys_isr_exit_i(
    void) {

    NREQUIRE(NAPI_USAGE, NSCHED_INIT > global_sched_ctx.state);

    PORT_ISR_EXIT();

    if (true == PORT_ISR_IS_LAST()) {
        ((struct nsched_ctx *)&global_sched_ctx)->state &= ~DEF_SCHED_STATE_INTSRV_MSK;
        nsched_yield_isr_i();
    }
}

/*--  Thread management  -----------------------------------------------------*/

void nthread_init(
    struct nthread *            thread,
    void                     (* fn)(void *),
    void *                      arg,
    struct nthread_stack *      stack,
    size_t                      stack_size,
    uint8_t                     priority)
{
    nintr_ctx                   intr_ctx;

    NREQUIRE(NAPI_USAGE,   global_sched_ctx.state > NSCHED_INACTIVE);
    NREQUIRE(NAPI_POINTER, thread != NULL);
    NREQUIRE(ES_API_OBJECT,  thread->signature != DEF_THD_CONTRACT_SIGNATURE);
    NREQUIRE(NAPI_POINTER, fn     != NULL);
    NREQUIRE(NAPI_POINTER, stack  != NULL);
    NREQUIRE(ES_API_RANGE,   stack_size >= PORT_STACK_MINSIZE);
    NREQUIRE(ES_API_RANGE,   priority < CONFIG_PRIORITY_LEVELS);
    NREQUIRE(ES_API_RANGE, ((&KVTmr != thread) && ((CONFIG_PRIORITY_LEVELS - 1u) > priority)) || (&KVTmr == thread));
    NREQUIRE(ES_API_RANGE, ((&KIdle != thread) && (0u < priority)) || (&KIdle == thread));
    NOBLIGATION(thread->signature = DEF_THD_CONTRACT_SIGNATURE);             /* Make thread structure valid.                             */

    thread->stack           = PORT_CTX_INIT(stack, stack_size, fn, arg);                       /* Make a fake thread stack.                                */
    thread->priority        = priority;
    thread->quantum_counter = CFG_SCHED_TIME_QUANTUM;
    thread->quantum_reload  = CFG_SCHED_TIME_QUANTUM;
    nthread_queue_init_entry(thread);
    NCRITICAL_LOCK_ENTER(&intr_ctx);
    nsched_init_thread_i(thread);                                                                   /* Initialize thread before adding it to Ready Thread queue.*/
    nsched_add_thread_i(thread);                                                                   /* Add the thread to Ready Thread queue.                    */
    nsched_yield_i();                                                            /* Invoke the scheduler.                                    */
    NCRITICAL_LOCK_EXIT(intr_ctx);

#if   (1u == CFG_HOOK_POST_THD_INIT)
    userPostThdInit();
#endif
}

void nthread_term(
    nthread *           thd) {

    nintr_ctx           intCtx;

    NREQUIRE(NAPI_USAGE, NSCHED_INACTIVE > global_sched_ctx.state);
    NREQUIRE(NAPI_POINTER, NULL != thd);
    NREQUIRE(ES_API_OBJECT, DEF_THD_CONTRACT_SIGNATURE == thd->signature);

#if   (1u == CFG_HOOK_PRE_THD_TERM)
    userPreThdTerm();
#endif
    NCRITICAL_LOCK_ENTER(&intCtx);

    nthread_queue_remove(thd);

    NOBLIGATION(thd->signature = ~DEF_THD_CONTRACT_SIGNATURE);            /* Mark the thread ID structure as invalid.                 */

    nsched_yield_i();
    NCRITICAL_LOCK_EXIT(intCtx);
}

void nthread_set_priority_i(
    nthread *           thd,
    uint8_t             prio) {

    NREQUIRE(NAPI_USAGE, NSCHED_INACTIVE > global_sched_ctx.state);
    NREQUIRE(NAPI_POINTER, NULL != thd);
    NREQUIRE(ES_API_OBJECT, DEF_THD_CONTRACT_SIGNATURE == thd->signature);
    NREQUIRE(ES_API_RANGE, CONFIG_PRIORITY_LEVELS >= prio);

    struct nthread_queue * queue;

    queue = nthread_queue_get_container(thd);

    if (queue == NULL) {                             /* Is thread inserted in any queue?                         */
        thd->priority = prio;                                                       /* Just change it's priority value.                             */
    } else {
        nthread_queue_remove(thd);
        thd->priority = prio;
        nthread_queue_insert(queue, thd);
        nsched_evaluate_i();
    }
}

/*--  Virtual Timer management  ----------------------------------------------*/

void esKTmrInitI(
    esVTmr_T *          vTmr,
    esVTmrTick            tick,
    void (* fn)(void *),
    void *              arg) {

    NREQUIRE(NAPI_USAGE, NSCHED_INACTIVE > global_sched_ctx.state);
    NREQUIRE(NAPI_POINTER, NULL != vTmr);
    NREQUIRE(ES_API_OBJECT, DEF_VTMR_CONTRACT_SIGNATURE != vTmr->signature);
    NREQUIRE(ES_API_RANGE, 1u < tick);
    NREQUIRE(NAPI_POINTER, NULL != fn);

    vTmr->rtick     = tick;
    vTmr->fn        = fn;
    vTmr->arg       = arg;
    vTmr->tmrL.q    = &VTmrPend;
    NDLIST_ADD_AFTER(tmrL, &VTmrPend, vTmr);
    ++SysTmr.vTmrPend;

#if   (1u == CFG_SYSTMR_ADAPTIVE_MODE)
    if (0u != SysTmr.ptick) {                                                   /* If system is sleeping we need to wake up VTmr thread.    */
        esThdPostI(
            &KVTmr);
    }
#endif
    NOBLIGATION(vTmr->signature = DEF_VTMR_CONTRACT_SIGNATURE);
}

void esVTmrInit(
    esVTmr_T *          vTmr,
    esVTmrTick            tick,
    void (* fn)(void *),
    void *              arg) {

    nintr_ctx           intCtx;

    NCRITICAL_LOCK_ENTER(&intCtx);
    esKTmrInitI(
        vTmr,
        tick,
        fn,
        arg);
    NCRITICAL_LOCK_EXIT(intCtx);
}

void esVTmrTermI(
    esVTmr_T *          vTmr) {

    NREQUIRE(NAPI_USAGE, NSCHED_INACTIVE > global_sched_ctx.state);
    NREQUIRE(NAPI_POINTER, NULL != vTmr);
    NREQUIRE(ES_API_OBJECT, DEF_VTMR_CONTRACT_SIGNATURE == vTmr->signature);

    NOBLIGATION(vTmr->signature = ~DEF_VTMR_CONTRACT_SIGNATURE);

    if (&VTmrPend == vTmr->tmrL.q) {                                            /* A pending timer is being deleted.                        */
        NDLIST_REMOVE(tmrL, vTmr);
        --SysTmr.vTmrPend;
    } else {                                                                    /* An armed timer is being deleted.                         */

#if   (0u == CFG_SYSTMR_ADAPTIVE_MODE)
        esVTmr_T *      nextVTmr;

        --SysTmr.vTmrArmed;
        nextVTmr = NDLIST_NEXT(tmrL, vTmr);

        if (&VTmrArmed != nextVTmr) {
            nextVTmr->rtick += vTmr->rtick;
        }
        NDLIST_REMOVE(tmrL, vTmr);
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

    nintr_ctx           intrCtx;

    NCRITICAL_LOCK_ENTER(&intrCtx);
    esVTmrTermI(
        vTmr);
    NCRITICAL_LOCK_EXIT(intrCtx);
}

void esVTmrDelay(
    esVTmrTick            tick) {

    esVTmr_T            vTmr;

    esVTmrInit(
        &vTmr,
        tick,
        (void (*)(void *))thdPost,
        (void *)nsched_get_current());
    thdWait();
}

/*--  Kernel time management  ------------------------------------------------*/

esVTmrTick esSysTmrTickGet(
    void) {

    esVTmrTick            tick;

    NREQUIRE(NAPI_USAGE, NSCHED_INACTIVE > global_sched_ctx.state);

#if   (0u == CFG_SYSTMR_ADAPTIVE_MODE)
# if   (PORT_DATA_SIZE_VAL >= SYSTMR_TICK_TYPE_SIZE)
    tick = SysTmr.ctick;

    return (tick);
# else
    {
        nintr_ctx       intCtx;

        NCRITICAL_LOCK_ENTER(&intCtx);
        tick = SysTmr.ctick;
        NCRITICAL_LOCK_EXIT(intCtx);

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
