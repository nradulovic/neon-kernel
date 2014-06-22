/*
 * This file is part of NUB Real-Time Kernel.
 *
 * Copyright (C) 2010 - 2014 Nenad Radulovic
 *
 * NUB Real-Time Kernel is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * NUB Real-Time Kernel is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with NUB Real-Time Kernel.  If not, see <http://www.gnu.org/licenses/>.
 *
 * web site:    http://github.com/nradulovic
 * e-mail  :    nenad.b.radulovic@gmail.com
 *//***************************************************************************************************************//**
 * @file
 * @author      Nenad Radulovic
 * @brief       Kernel implementation
 * @addtogroup  kernel
 *************************************************************************************************************//** @{ */
/**@defgroup    kernel_impl Implementation
 * @brief       Implementation
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

/*=================================================================================================  INCLUDE FILES  ==*/

#include "plat/critical.h"
#include "arch/intr.h"
#include "kernel/nub.h"

/*=================================================================================================  LOCAL MACRO's  ==*/

/*----------------------------------------------------------------------------------------------------------------*//**
 * @name        Generic helper macros
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

#define TO_THREAD(list)                     container_of(list, struct nthread, array_entry)

/**@} *//*--------------------------------------------------------------------------------------------------------*//**
 * @defgroup    prio_array Priority array
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

#define PRIO_ARRAY_BUCKET_BITS              N_LOG2_8(NDIVISION_ROUNDUP(CONFIG_PRIORITY_LEVELS, CONFIG_PRIORITY_BUCKETS))

/**@} *//*--------------------------------------------------------------------------------------------------------*//**
 * @name        Threads
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

/**@brief       Thread structure signature.
 * @details     The signature is used to confirm that a structure passed to a function is indeed a nthread thread
 *              structure.
 */
#define THREAD_SIGNATURE                    ((natomic)0xfeedbeeful)

/**@} *//*--------------------------------------------------------------------------------------------------------*//**
 * @name        Semaphore
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

/**@brief       Timer structure signature.
 * @details     The signature is used to confirm that a structure passed to a semaphore function is indeed a semaphore
 *               structure.
 */
#define SEM_SIGNATURE                       ((natomic)0xfeedbef0ul)

/**@} *//*--------------------------------------------------------------------------------------------------------*//**
 * @name        Timer
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

/**@brief       Timer structure signature.
 * @details     The signature is used to confirm that a structure passed to a timer function is indeed a timer structure.
 */
#define TIMER_SIGNATURE                     ((natomic)0xfeedbef1ul)

/**@} *//*------------------------------------------------------------------------------------------------------------*/

/*==============================================================================================  LOCAL DATA TYPES  ==*/

/*----------------------------------------------------------------------------------------------------------------*//**
 * @name        System timer
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

/**@brief       Main System Timer structure
 * @note        1) Member `ptick` exists only if ADAPTIVE mode is selected. When
 *              this mode is selected then kernel supports more aggressive power
 *              savings.
 */
struct sys_timer
{
    uint_fast16_t       vTmrArmed;                                              /**< @brief The number of armed virtual timers in system.   */
    uint_fast16_t       vTmrPend;                                               /**< @brief The number of pending timers for arming.        */
    //esVTmrTick            ctick;                                                  /**< @brief Current system timer tick value.                */
#if   (1u == CFG_SYSTMR_ADAPTIVE_MODE) || defined(__DOXYGEN__)
    esVTmrTick            ptick;                                                  /**< @brief Pending ticks during the timer sleep mode.      */
#endif
};

/**@} *//*------------------------------------------------------------------------------------------------------------*/


/*=====================================================================================  LOCAL FUNCTION PROTOTYPES  ==*/

/*----------------------------------------------------------------------------------------------------------------*//**
 * @name        Priority array
 * @{ *//*------------------------------------------------------------------------------------------------------------*/
#if (CONFIG_PRIORITY_BUCKETS != 1)


/**@brief       Initialize bitmap
 * @param       bitmap
 *              Pointer to the bit map structure
 */
static PORT_C_INLINE void bitmap_init(
    struct nub_prio_bitmap *    bitmap);



/**@brief       Set the bit corresponding to the priority argument
 * @param       bitmap
 *              Pointer to the bit map structure
 * @param       priority
 *              Priority which will be marked as used
 */
static PORT_C_INLINE void bitmap_set(
    struct nub_prio_bitmap *    bitmap,
    uint_fast8_t                priority);



/**@brief       Clear the bit corresponding to the priority argument
 * @param       bitmap
 *              Pointer to the bit map structure
 * @param       priority
 *              Priority which will be marked as unused
 */
static PORT_C_INLINE void bitmap_clear(
    struct nub_prio_bitmap *    bitmap,
    uint_fast8_t                priority);



/**@brief       Get the highest priority set
 * @param       bitmap
 *              Pointer to the bit map structure
 * @return      The number of the highest priority marked as used
 */
static PORT_C_INLINE uint_fast8_t bitmap_get_highest(
    const struct nub_prio_bitmap * bitmap);



static bool bitmap_is_empty(
    const struct nub_prio_bitmap * bitmap);

#endif  /* (CONFIG_PRIORITY_BUCKETS != 1) */


static void prio_array_init(
    struct nub_prio_array *     array);



static void prio_array_insert(
    struct nthread *            thread,
    struct nub_prio_array *     array);



static void prio_array_remove(
    struct nthread *            thread,
    struct nub_prio_array *     array);



static struct nthread * prio_array_peek(
    const struct nub_prio_array * array);



static struct nthread * prio_array_rotate_thread(
    struct nthread *            thread,
    struct nub_prio_array *     array);



static void prio_array_init_entry(
    struct nthread *            thread);



static bool prio_array_is_empty(
    const struct nub_prio_array * array);

/**@} *//*--------------------------------------------------------------------------------------------------------*//**
 * @name        FIFO array
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

/**@} *//*--------------------------------------------------------------------------------------------------------*//**
 * @name        Scheduler
 * @{ *//*------------------------------------------------------------------------------------------------------------*/


/**@brief       Initialize Ready Thread Queue structure @ref RdyQueue and Scheduler context structure
 *              @ref global_nub_sys.
 */
static void sched_init(
    void);



/**@brief       Set the scheduler data structures for multi-threading
 * @details     This function is called just before multi-threading will start.
 */
static void sched_start(
    void);



/**@brief       Initialize scheduler ready structure during the thread add operation
 * @param       thread
 *              Pointer to the thread currently being initialized.
 * @details     Function will initialize scheduler structures during the initialization phase of the kernel.
 */
static void sched_register_thread_i(
    struct nthread *            thread);



/**@brief       Add thread `thread` to the ready thread array_entry and notify the scheduler.
 * @param       thread
 *              Pointer to the initialized thread ID structure, @ref nthread.
 */
static void sched_insert_thread_i(
    struct nthread *            thread);



/**@brief       Remove thread `thread` from the ready thread array_entry and notify the scheduler.
 * @param       thread
 *              Pointer to the initialized thread ID structure, @ref esThread.
 */
static void sched_remove_thread_i(
    struct nthread *            thread);



/**@brief       Force the scheduler invocation which will evaluate all ready threads and switch to ready thread with the
 *              highest priority
 */
static void sched_reschedule_i(
    void);



/**@brief       Enter Interrupt Service Routine
 */
static void sched_isr_enter_i(
    void);



/**@brief       Exit Interrupt Service Routine
 */
static void sched_isr_exit_i(
    void);



/**@brief       Lock the scheduler
 */
static void sched_lock_enter_i(
    void);



/**@brief       Unlock the scheduler
 */
static void sched_lock_exit_i(
    void);



/**@brief       Get the current thread ID
 * @return      Pointer to current thread ID structure @ref nthread.
 */
static struct nthread * sched_get_current(
    void);



/**@brief       Set the scheduler to sleep
 * @note        This function is used only when @ref CFG_SCHED_POWER_SAVE option
 *              is active.
 */
#if   (1u == CFG_SCHED_POWER_SAVE)
static void sched_sleep(
    void);
#endif



/**@brief       Wake up the scheduler
 * @note        This function is used only when @ref CFG_SCHED_POWER_SAVE option
 *              is active.
 */
#if   (1u == CFG_SCHED_POWER_SAVE)
static void sched_wake_up_i(
    void);
#endif



/**@brief       Do the Quantum (Round-Robin) scheduling
 */
static void sched_quantum_i(
    void);

/**@} *//*------------------------------------------------------------------------------------------------------------*/

/*===============================================================================================  LOCAL VARIABLES  ==*/
/*==============================================================================================  GLOBAL VARIABLES  ==*/

/*----------------------------------------------------------------------------------------------------------------*//**
 * @name        Scheduler
 * @{ *//*------------------------------------------------------------------------------------------------------------*/

struct nub_sys_ctx              global_nub_sys =
{
    .state = NSYS_INACTIVE                                                      /* This is the default kernel state.  */
};

/**@} *//*------------------------------------------------------------------------------------------------------------*/

/*====================================================================================  LOCAL FUNCTION DEFINITIONS  ==*/


/*--  Priority array bitmap  -----------------------------------------------------------------------------------------*/
#if (CONFIG_PRIORITY_BUCKETS != 1)


static PORT_C_INLINE void bitmap_init(
    struct nub_prio_bitmap *    bitmap)
{

#if   (CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH)
    uint_fast8_t                group;

    bitmap->bitGroup = 0u;
    group = NARRAY_DIMENSION(bitmap->bit);

    while (group-- != 0u)
    {
        bitmap->bit[group] = 0u;
    }
#else   /*  (CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH) */
    bitmap->bit[0] = 0u;
#endif  /* !(CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH) */
}



static PORT_C_INLINE void bitmap_set(
    struct nub_prio_bitmap *    bitmap,
    uint_fast8_t                priority)
{

#if   (CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH)
    uint_fast8_t                group_index;
    uint_fast8_t                bit_index;

    bit_index   = priority & (~((uint_fast8_t)0u) >> (sizeof(priority) * 8u - N_LOG2_8(NCPU_DATA_WIDTH)));
    group_index = priority >> N_LOG2_8(NCPU_DATA_WIDTH);
    bitmap->bitGroup         |= NCPU_EXP2(group_index);
    bitmap->bit[group_index] |= NCPU_EXP2(bit_index);
#else   /*  (CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH) */
    bitmap->bit[0] |= NCPU_EXP2(priority);
#endif  /* !(CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH) */
}



static PORT_C_INLINE void bitmap_clear(
    struct nub_prio_bitmap *    bitmap,
    uint_fast8_t                priority)
{

#if   (CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH)
    uint_fast8_t                group_index;
    uint_fast8_t                bit_index;

    bit_index   = priority & (~((uint_fast8_t)0u) >> (sizeof(priority) * 8u - N_LOG2_8(NCPU_DATA_WIDTH)));
    group_index = priority >> N_LOG2_8(NCPU_DATA_WIDTH);
    bitmap->bit[group_index] &= ~NCPU_EXP2(bit_index);

    if (bitmap->bit[group_index] == 0u)                                         /* If this is the last bit cleared in */
    {                                                                           /* this array_entry then clear bit    */
        bitmap->bitGroup &= ~NCPU_EXP2(group_index);                            /* group indicator, too.              */
    }
#else   /*  (CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH) */
    bitmap->bit[0] &= ~NCPU_EXP2(priority);
#endif  /* !(CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH) */
}



static PORT_C_INLINE uint_fast8_t bitmap_get_highest(
    const struct nub_prio_bitmap * bitmap)
{
#if   (CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH)
    uint_fast8_t                group_index;
    uint_fast8_t                bit_index;

    group_index = NCPU_FIND_LAST_SET(bitmap->bitGroup);
    bit_index   = NCPU_FIND_LAST_SET(bitmap->bit[group_index]);

    return ((group_index << N_LOG2_8(NCPU_DATA_WIDTH)) | bit_index);
#else   /*  (CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH) */
    uint_fast8_t                bit_index;

    bit_index = NCPU_FIND_LAST_SET(bitmap->bit[0]);

    return (bit_index);
#endif  /* !(CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH) */
}



static bool bitmap_is_empty(
    const struct nub_prio_bitmap * bitmap)
{
#if   (CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH)
    if (bitmap->bitGroup == 0u)
    {
        return (true);
    }
    else
    {
        return (false);
    }
#else   /*  (CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH) */
    if (bitmap->bit == 0u)
    {
        return (true);
    }
    else
    {
        return (false);
    }
#endif  /* !(CONFIG_PRIORITY_BUCKETS > NCPU_DATA_WIDTH) */
}

#endif  /* (CONFIG_PRIORITY_BUCKETS != 1) */


/*--  Priority array  ------------------------------------------------------------------------------------------------*/


static void prio_array_init(
    struct nub_prio_array *     array)
{
    uint_fast8_t                count;

#if (CONFIG_PRIORITY_BUCKETS != 1)
    bitmap_init(&array->bitmap);
#endif
    count = NARRAY_DIMENSION(array->sentinel);

    while (count-- != 0u)                                                       /* Initialize each list entry.        */
    {
        ndlist_init(&array->sentinel[count]);
    }
}



static void prio_array_insert(
    struct nthread *            thread,
    struct nub_prio_array *     array)
{
    uint_fast8_t                bucket;
    struct ndlist *             entry;

#if (CONFIG_PRIORITY_BUCKETS != 1)
    bucket = thread->priority >> PRIO_ARRAY_BUCKET_BITS;
#else
    bucket = 0u;
#endif
    entry = &array->sentinel[bucket];                                           /* entry iterates over the array.     */
#if (CONFIG_PRIORITY_BUCKETS != 1)

    if (ndlist_is_empty(entry))                                                 /* If adding the first entry in list. */
    {
        bitmap_set(&array->bitmap, bucket);                                     /* Mark the bucket list as used.      */
    }
#endif
#if (CONFIG_PRIORITY_BUCKETS != CONFIG_PRIORITY_LEVELS)                         /* Priority search and insertion.     */
    do
    {
        entry = ndlist_next(entry);                                             /* Iterate to next thread in array.   */
    }
    while ((entry != &array->sentinel[bucket]) && (TO_THREAD(entry)->priority >= thread->priority));
                                                                                /* Not end of array and entry has     */
                                                                                /* equal or higher priority than      */
                                                                                /* thread?                            */
#endif
    ndlist_add_after(entry, &thread->array_entry);
}



static void prio_array_remove(
    struct nthread *            thread,
    struct nub_prio_array *     array)
{
#if (CONFIG_PRIORITY_BUCKETS != 1)
    uint_fast8_t                bucket;

    bucket = thread->priority >> PRIO_ARRAY_BUCKET_BITS;
#else
    (void)array;
#endif
    ndlist_remove(&thread->array_entry);
#if (CONFIG_PRIORITY_BUCKETS != 1)

    if (ndlist_is_empty(&array->sentinel[bucket]))                              /* If this was the last entry in list.*/
    {
        bitmap_clear(&array->bitmap, bucket);                                   /* Mark the bucket as unused.         */
    }
#endif
}



static struct nthread * prio_array_peek(
    const struct nub_prio_array * array)
{
    uint_fast8_t                bucket;

#if (CONFIG_PRIORITY_BUCKETS != 1)
    bucket = bitmap_get_highest(&array->bitmap);
#else
    bucket = 0u;
#endif

    return (TO_THREAD(array->sentinel[bucket].next));
}



static struct nthread * prio_array_rotate_thread(
    struct nthread *            thread,
    struct nub_prio_array *     array)
{
    uint_fast8_t                bucket;

#if (CONFIG_PRIORITY_BUCKETS != 1)
    bucket = thread->priority >> PRIO_ARRAY_BUCKET_BITS;
#else
    bucket = 0u;
#endif
#if (CONFIG_PRIORITY_BUCKETS != CONFIG_PRIORITY_LEVELS)
    {
        struct ndlist *        entry;

        ndlist_remove(&thread->array_entry);                                    /* Remove thread from bucket.         */
        entry = &array->sentinel[bucket];                                       /* entry iterates over array.         */
        do
        {
            entry = ndlist_next(entry);
        }
        while ((entry != &array->sentinel[bucket]) && (TO_THREAD(entry)->priority >= thread->priority));
        ndlist_add_after(entry, &thread->array_entry);                          /* Insert the thread at new position. */
    }
#else
    ndlist_remove(&thread->array_entry);
    ndlist_add_before(&array->sentinel[bucket], &thread->array_entry);
#endif

    return (TO_THREAD(array->sentinel[bucket].next));
}



static void prio_array_init_entry(
    struct nthread *            thread)
{
    ndlist_init(&thread->array_entry);
}



static bool prio_array_is_empty(
    const struct nub_prio_array * array)
{
    return (bitmap_is_empty(&array->bitmap));
}

/*--  Scheduler  -----------------------------------------------------------------------------------------------------*/


static void sched_init(
    void)
{
    prio_array_init(&global_nub_sys.run_queue);                                 /* Initialize run_queue structure.    */
    global_nub_sys.state = NSYS_INIT;
}



static void sched_start(
    void)
{
    struct nthread *            new_thread;
    nintr_ctx                   intr_ctx;

    NCRITICAL_LOCK_ENTER(&intr_ctx);
    new_thread = prio_array_peek(&global_nub_sys.run_queue);                    /* Get the highest priority thread.   */
    global_nub_sys.cthread = new_thread;
    global_nub_sys.pthread = new_thread;
    global_nub_sys.state   = NSYS_RUN;
    NCRITICAL_LOCK_EXIT(intr_ctx);
    NPORT_DISPATCH_TO_FIRST();                                                  /* Start the first thread.            */
}



static void sched_register_thread_i(
    struct nthread *            thread)
{
    if (global_nub_sys.pthread == NULL)                                         /* If this is the first thread created*/
    {
        global_nub_sys.pthread = thread;                                        /* then make it pending.              */
    }
    else if (global_nub_sys.pthread->priority < thread->priority)
    {
        global_nub_sys.pthread = thread;
    }
    prio_array_insert(thread, &global_nub_sys.run_queue);
}



static void sched_insert_thread_i(
    struct nthread *            thread)
{
    prio_array_insert(thread, &global_nub_sys.run_queue);

    if (global_nub_sys.pthread->priority < thread->priority)
    {
        global_nub_sys.pthread = thread;
    }
}



static void sched_remove_thread_i(
    struct nthread *            thread)
{
    prio_array_remove(thread, &global_nub_sys.run_queue);

    if (global_nub_sys.pthread == thread)
    {                                                                           /* Get new highest priority thread.   */
        global_nub_sys.pthread = prio_array_peek(&global_nub_sys.run_queue);
    }
}



static void sched_reschedule_i(
    void)
{
    if (global_nub_sys.state == NSYS_RUN)                                       /* Context switching is needed only   */
    {                                                                           /* when cthread and pthread are       */
        if (global_nub_sys.cthread != global_nub_sys.pthread)                   /* different.                         */
        {
#if   (1u == CFG_HOOK_PRE_CTX_SW)
            userPreCtxSw(global_nub_sys.cthread, newThd);
#endif
            PORT_DISPATCH();
        }
    }
}



static void sched_isr_enter_i(
    void)
{
    global_nub_sys.state |= NSYS_ISR;
}



static void sched_isr_exit_i(
    void)
{
    if (global_nub_sys.state == NSYS_ISR)
    {
        global_nub_sys.state &= ~NSYS_ISR;

        if (global_nub_sys.cthread != global_nub_sys.pthread)                   /* Context switching is needed only   */
        {                                                                       /* when cthread and pthread are       */
                                                                                /* different.                         */
#if   (1u == CFG_HOOK_PRE_CTX_SW)
            userPreCtxSw(global_nub_sys.cthread, global_nub_sys.pthread);
#endif
            PORT_DISPATCH_ISR();
#if   (1u == CFG_SCHED_POWER_SAVE)
        }
        else if (NSYS_SLEEP == global_nub_sys.state)
        {
            sched_wake_up_i();
#endif
        }
    }
}



static void sched_lock_enter_i(
    void)
{
    global_nub_sys.state |= NSYS_LOCK;
    global_nub_sys.lock_count++;
}



static void sched_lock_exit_i(
    void)
{
    global_nub_sys.lock_count--;

    if (global_nub_sys.lock_count == 0u)
    {
        global_nub_sys.state &= ~NSYS_LOCK;
        sched_reschedule_i();
    }
}



static struct nthread * sched_get_current(
    void)
{
    return (global_nub_sys.cthread);
}

#if   (1u == CFG_SCHED_POWER_SAVE) || defined(__DOXYGEN__)


static void sched_sleep(
    void) {

    nintr_ctx           intrCtx;

    NCRITICAL_LOCK_ENTER(&intrCtx);
    sched_lock_enter_i();
# if (1u == CFG_HOOK_PRE_IDLE)
    userPreIdle();
# endif
    global_nub_sys.state = NSYS_SLEEP;
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
    NCRITICAL_LOCK_EXIT(intrCtx);
# endif
    NCRITICAL_LOCK_ENTER(&intrCtx);
    } while (global_nub_sys.cthread == global_nub_sys.pthread);
# if (1u == CFG_HOOK_POST_IDLE)
    userPostIdle();
# endif
    sched_lock_exit_i();
    NCRITICAL_LOCK_EXIT(intrCtx);
}

#endif
#if   (1u == CFG_SCHED_POWER_SAVE) || defined(__DOXYGEN__)


static void sched_wake_up_i(
    void) {

    global_nub_sys.state = NSYS_LOCK;
# if (1u == CFG_SYSTMR_ADAPTIVE_MODE)
    sysTmrActivate();                                                           /* Switch to normal system timer operation.                 */
# endif
}

#endif


static void sched_quantum_i(
    void)
{
    if (!(global_nub_sys.state & NSYS_LOCK))                                    /* Round-Robin is not enabled in      */
    {                                                                           /* kernel LOCK state                  */
        struct nthread * thread;

        thread = global_nub_sys.cthread;                                        /* Get the current thread             */
        thread->quantum_counter--;                                              /* Decrement current thread time      */
                                                                                /* quantum                            */
        if (thread->quantum_counter == 0u)
        {
            thread->quantum_counter = thread->quantum_reload;                   /* Reload thread time quantum         */
            thread = prio_array_rotate_thread(thread, &global_nub_sys.run_queue);   /* Fetch the next thread and      */
                                                                                /* rotate this priority array_entry   */
            if (global_nub_sys.pthread == global_nub_sys.cthread)               /* If there is no thread pending      */
            {
                global_nub_sys.pthread = thread;                                /* Make the new thread pending        */
            }
        }
    }
}

/*===========================================================================  GLOBAL PRIVATE FUNCTION DEFINITIONS  ==*/
/*============================================================================  GLOBAL PUBLIC FUNCTION DEFINITIONS  ==*/

/*--  Kernel generic functions  --------------------------------------------------------------------------------------*/


void nsys_init(
    void) {

    NREQUIRE(NAPI_USAGE, NSYS_INACTIVE == global_nub_sys.state);
#if   (1u == CFG_HOOK_PRE_KERN_INIT)
    userPreKernInit();
#endif
    ES_INTR_DISABLE();
    PORT_KCORE_INIT_EARLY();
    sched_init();
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

    NREQUIRE(NAPI_USAGE, NSYS_INIT == global_nub_sys.state);

#if   (1u == CFG_HOOK_PRE_KERN_START)
    userPreKernStart();
#endif
    PORT_KCORE_INIT_LATE();
    sched_start();                                                              /* Initialize scheduler.              */

    while (true);                                                               /* Prevent compiler warnings.         */
}



void nub_sys_timer_isr(
    void) {

    nintr_ctx           intr_ctx;

    NREQUIRE(NAPI_USAGE, global_nub_sys.cthread != &KVTmr);

#if   (1u == CFG_HOOK_PRE_SYSTMR_EVENT)
    userPreSysTmr();
#endif
    NCRITICAL_LOCK_ENTER(&intr_ctx);
    sched_quantum_i();
    NCRITICAL_LOCK_EXIT(intr_ctx);
}



void nsys_isr_enter_i(
    void) {

    NREQUIRE(NAPI_USAGE, NSYS_INIT > global_nub_sys.state);

    PORT_ISR_ENTER();
    sched_isr_enter_i();
}



void nsys_isr_exit_i(
    void) {

    NREQUIRE(NAPI_USAGE, NSYS_INIT > global_nub_sys.state);

    PORT_ISR_EXIT();

    if (NPORT_IS_ISR_LAST())
    {
        sched_isr_exit_i();
    }
}



void nsys_lock_enter(
    void)
{
    nintr_ctx                   intr_ctx;

    NCRITICAL_LOCK_ENTER(&intr_ctx);
    sched_lock_enter_i();
    NCRITICAL_LOCK_EXIT(intr_ctx);
}



void nsys_lock_exit(
    void)
{
    nintr_ctx                   intr_ctx;

    NCRITICAL_LOCK_ENTER(&intr_ctx);
    sched_lock_exit_i();
    NCRITICAL_LOCK_EXIT(intr_ctx);
}



void nsys_lock_int_enter(
    nintr_ctx *                 intr_ctx)
{
    NCRITICAL_LOCK_ENTER(intr_ctx);
    sched_lock_enter_i();
}



void nsys_lock_int_exit(
    nintr_ctx                   intr_ctx)
{
    sched_lock_exit_i();
    NCRITICAL_LOCK_EXIT(intr_ctx);
}


/*--  Threads functions  ---------------------------------------------------------------------------------------------*/


void nthread_init(
    struct nthread *            thread,
    void                     (* entry)(void *),
    void *                      arg,
    struct nthread_stack *      stack,
    size_t                      stack_size,
    uint8_t                     priority)
{
    nintr_ctx                   intr_ctx;

    NREQUIRE(NAPI_POINTER, thread != NULL);
    NREQUIRE(NAPI_OBJECT,  thread->signature != THREAD_SIGNATURE);
    NREQUIRE(NAPI_POINTER, entry  != NULL);
    NREQUIRE(NAPI_POINTER, stack  != NULL);
    NREQUIRE(NAPI_RANGE,   stack_size >= PORT_STACK_MINSIZE);
    NREQUIRE(NAPI_RANGE,   priority < CONFIG_PRIORITY_LEVELS);
    NOBLIGATION(thread->signature = THREAD_SIGNATURE);                          /* Validate thread structure          */

    thread->stack           = PORT_CTX_INIT(stack, stack_size, entry, arg);     /* Make a fake thread stack           */
    thread->priority        = priority;
    thread->opriority       = priority;
    thread->quantum_counter = CONFIG_SCHED_TIME_QUANTUM;
    thread->quantum_reload  = CONFIG_SCHED_TIME_QUANTUM;
    NCRITICAL_LOCK_ENTER(&intr_ctx);
    sched_register_thread_i(thread);                                            /* Register and add to Run Queue      */
    sched_reschedule_i();                                                       /* Invoke the scheduler               */
    NCRITICAL_LOCK_EXIT(intr_ctx);

#if   (1u == CFG_HOOK_POST_THD_INIT)
    userPostThdInit();
#endif
}



void nthread_term(
    void)
{
    nintr_ctx                   intr_ctx;
    struct nthread *            thread;

    NREQUIRE(NAPI_POINTER, thread != NULL);
    NREQUIRE(NAPI_OBJECT,  thread->signature != THREAD_SIGNATURE);
    NOBLIGATION(thread->signature = ~THREAD_SIGNATURE);                         /* Mark the structure as invalid.     */

#if   (1u == CFG_HOOK_PRE_THD_TERM)
    userPreThdTerm();
#endif
    NCRITICAL_LOCK_ENTER(&intr_ctx);
    thread = sched_get_current();
    sched_remove_thread_i(thread);
    sched_reschedule_i();
    NCRITICAL_LOCK_EXIT(intr_ctx);
}



void nthread_set_priority(
    uint8_t                     priority)
{
    nintr_ctx                   intr_ctx;
    struct nthread *            thread;

    NREQUIRE(NAPI_RANGE,   priority < CONFIG_PRIORITY_LEVELS);

    NCRITICAL_LOCK_ENTER(&intr_ctx);
    thread = sched_get_current();
    sched_remove_thread_i(thread);
    thread->priority  = priority;
    thread->opriority = priority;
    sched_insert_thread_i(thread);
    sched_reschedule_i();
    NCRITICAL_LOCK_EXIT(intr_ctx);
}


/*--  Semaphore functions  -------------------------------------------------------------------------------------------*/


void nsem_init(
    struct nsem *               sem,
    uint32_t                    count)
{
    prio_array_init(&sem->prio_array);
    sem->count = count;
}



void nsem_term(
    struct nsem *               sem)
{
    nintr_ctx                   intr_ctx;

    NCRITICAL_LOCK_ENTER(&intr_ctx);

    while (!prio_array_is_empty(&sem->prio_array))
    {
        struct nthread *        thread;

        thread = prio_array_peek(&sem->prio_array);
        thread->op_status = N_E_OBJ_REMOVED;
        prio_array_remove(thread, &sem->prio_array);
        sched_insert_thread_i(thread);
    }
    sched_reschedule_i();
    NCRITICAL_LOCK_EXIT(intr_ctx);
}



enum nsys_status nsem_wait(
    struct nsem *               sem)
{
    nintr_ctx                   intr_ctx;
    struct nthread *            thread;

    NCRITICAL_LOCK_ENTER(&intr_ctx);
    thread = sched_get_current();

    if (sem->count != 0)
    {
        sem->count--;
    }
    else
    {
        sched_remove_thread_i(thread);
        prio_array_insert(thread, &sem->prio_array);
        sched_reschedule_i();

        if (thread->op_status == N_SUCCESS)
        {
            sem->count--;
        }
    }
    NCRITICAL_LOCK_EXIT(intr_ctx);

    return (thread->op_status);
}



void nsem_signal(
    struct nsem *               sem)
{
    nintr_ctx                   intr_ctx;

    NCRITICAL_LOCK_ENTER(&intr_ctx);
    sem->count++;
}

/*========================================================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//** @} *//*************************************************************************************
 * END of nub.c
 **********************************************************************************************************************/
