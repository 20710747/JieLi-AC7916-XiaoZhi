/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** GUIX Component                                                        */
/**                                                                       */
/**   System Management (System)                                          */
/**                                                                       */
/**************************************************************************/

//#define GX_SOURCE_CODE

/* Include necessary system files.  */

#include "gx_api.h"
#include "gx_system.h"
#include "gx_system_rtos_bind.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _gx_system_rtos_bind                                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Kenneth Maxwell, Microsoft Corporation                              */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file contains a small set of shell functions used to bind GUIX */
/*    to a generic RTOS. GUIX is by default configured to run with the    */
/*    ThreadX RTOS, but you can port to another RTOS by defining the      */
/*    pre-processor directive GX_DISABLE_THREADX_BINDING and completing   */
/*    the implementation of the functions found in this file.             */
/*    Refer to the GUIX User Guide for more information.                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    Refer to GUIX User Guide                                            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Refer to GUIX User Guide                                            */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    Refer to GUIX User Guide                                            */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    GUIX system serives                                                 */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Kenneth Maxwell          Initial Version 6.0           */
/*  09-30-2020     Kenneth Maxwell          Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/

#ifndef GX_THREADX_BINDING

/* This is an example of a generic RTOS binding. We have #ifdefed this code out
   of the GUIX library build, so that the user attempting to port to a new RTOS
   will intentionally observe a series of linker errors. These functions need to
   be implemented in a way that works correctly with the target RTOS.
*/
#if defined(CONFIG_FREE_RTOS_ENABLE)
#include "os/os_api.h"


#define GX_TIMER_TASK_PRIORITY   26
#define GX_TIMER_TASK_STACK_SIZE 512
void guix_timer_task_entry(void *);
static char guix_timer_task_run_flag;
static OS_SEM guix_timer_task_run_sem;

/* a few global status variables */
GX_BOOL guix_timer_event_pending;


/* define semaphore for lock/unlock macros */
OS_MUTEX guix_system_lock_mutex;

/* a custom event structure, to hold event and linked list */

typedef struct guix_linked_event_struct {
    GX_EVENT                         event_data;        /* the GUIX event structure */
    struct guix_linked_event_struct *next;              /* add link member          */
} GUIX_LINKED_EVENT;


/* a custom fifo event queue structure */

typedef struct guix_event_queue_struct {
    GUIX_LINKED_EVENT *first;           /* first (oldest) event in fifo */
    GUIX_LINKED_EVENT *last;            /* last (newest) event in fifo  */
    GUIX_LINKED_EVENT *free;            /* linked list of free events   */
    GUIX_LINKED_EVENT *free_end;        /* end of the free list         */
    OS_SEM             count_sem;       /* number of queued events      */
    OS_MUTEX           lock;            /* lock to protect queue update */
} GUIX_EVENT_QUEUE;

/* an array of GX_EVENTs used to implement fifo queue */

GUIX_LINKED_EVENT guix_event_array[GX_MAX_QUEUE_EVENTS];
GUIX_EVENT_QUEUE  guix_event_queue;

/* rtos initialize: perform any setup that needs to be done before the GUIX task runs here */
VOID   gx_generic_rtos_initialize(VOID)
{
    int Loop;
    GUIX_LINKED_EVENT *current;

    guix_timer_event_pending = GX_FALSE;
    os_mutex_create(&guix_system_lock_mutex);

    /* initialize a custom fifo queue structure */

    guix_event_queue.first = GX_NULL;
    guix_event_queue.last = GX_NULL;
    guix_event_queue.free = guix_event_array;

    current = guix_event_queue.free;

    /* link all the structures together */
    for (Loop = 0; Loop < GX_MAX_QUEUE_EVENTS - 1; Loop++) {
        current -> next = (current + 1);
        current = current -> next;
    }
    current -> next = GX_NULL;                /* terminate the list */
    guix_event_queue.free_end = current;

    /* create mutex for lock access to this queue */
    os_mutex_create(&guix_event_queue.lock);

    /* create counting semaphore to track queue entries */
    os_sem_create(&guix_event_queue.count_sem, 0);
}

VOID (*gx_system_thread_entry)(ULONG);

// A small shell function to convert the void * arg expected by uC/OS to
// a ULONG parameter expected by GUIX thread entry
void gx_system_thread_entry_shell(void *parg)
{
    gx_system_thread_entry((ULONG) parg); //_gx_system_thread_entry
}

/* thread_start: start the GUIX thread running. */
UINT   gx_generic_thread_start(VOID(*guix_thread_entry)(ULONG))
{

    /* save the GUIX system thread entry function pointer */
    gx_system_thread_entry = guix_thread_entry;

    /* create the main GUIX task */

    static int  guix_pid;
    thread_fork("guix_task", GX_SYSTEM_THREAD_PRIORITY, GX_THREAD_STACK_SIZE, 0, &guix_pid, gx_system_thread_entry_shell, GX_NULL);

    /* create a simple task to generate timer events into GUIX */
    os_sem_create(&guix_timer_task_run_sem, 0);
    static int  guix_timer_pid;
    thread_fork("guix_timer_task", GX_TIMER_TASK_PRIORITY, GX_TIMER_TASK_STACK_SIZE, 0, &guix_timer_pid, guix_timer_task_entry, GX_NULL);

    /* suspend the timer task until needed */
    /*os_task_suspend("guix_timer_task");*/

    return GX_SUCCESS;
}

/* event_post: push an event into the fifo event queue */
UINT   gx_generic_event_post(GX_EVENT *event_ptr)
{
    GUIX_LINKED_EVENT *linked_event;

    /* lock down the guix event queue */
    os_mutex_pend(&guix_event_queue.lock, 0);

    /* grab a free event slot */
    if (!guix_event_queue.free) {
        /* there are no free events, return failure */
        os_mutex_post(&guix_event_queue.lock);
        return GX_FAILURE;
    }

    linked_event = guix_event_queue.free;
    guix_event_queue.free = guix_event_queue.free->next;

    if (!guix_event_queue.free) {
        guix_event_queue.free_end = GX_NULL;
    }

    /* copy the event data into this slot */
    linked_event -> event_data = *event_ptr;
    linked_event -> next = GX_NULL;

    /* insert this event into fifo queue */
    if (guix_event_queue.last) {
        guix_event_queue.last -> next = linked_event;
    } else {
        guix_event_queue.first = linked_event;
    }
    guix_event_queue.last = linked_event;

    /* Unlock the guix queue */
    os_mutex_post(&guix_event_queue.lock);

    /* increment event count */
    os_sem_post(&guix_event_queue.count_sem);
    return GX_SUCCESS;
}

/* event_fold: update existing matching event, otherwise post new event */
UINT   gx_generic_event_fold(GX_EVENT *event_ptr)
{
    GUIX_LINKED_EVENT *pTest;

    /* Lock down the guix queue */

    os_mutex_pend(&guix_event_queue.lock, 0);

    // see if the same event is already in the queue:
    pTest = guix_event_queue.first;

    while (pTest) {
        if (pTest -> event_data.gx_event_type == event_ptr -> gx_event_type) {
            /* found matching event, update it and return */
            pTest -> event_data.gx_event_payload.gx_event_ulongdata = event_ptr -> gx_event_payload.gx_event_ulongdata;
            os_mutex_post(&guix_event_queue.lock);
            return GX_SUCCESS;
        }
        pTest = pTest -> next;
    }

    /* did not find a match, push new event */

    os_mutex_post(&guix_event_queue.lock);
    gx_generic_event_post(event_ptr);
    return GX_SUCCESS;
}


/* event_pop: pop oldest event from fifo queue, block if wait and no events exist */
UINT   gx_generic_event_pop(GX_EVENT *put_event, GX_BOOL wait)
{
    if (!wait) {
        if (guix_event_queue.first == GX_NULL) {
            /* the queue is empty, just return */
            return GX_FAILURE;
        }
    }

    /* wait for an event to arrive in queue */
    os_sem_pend(&guix_event_queue.count_sem, 0);

    /* lock down the queue */
    os_mutex_pend(&guix_event_queue.lock, 0);

    /* copy the event into destination */
    *put_event = guix_event_queue.first -> event_data;

    /* link this event holder back into free list */
    if (guix_event_queue.free_end) {
        guix_event_queue.free_end -> next = guix_event_queue.first;
    } else {
        guix_event_queue.free = guix_event_queue.first;
    }

    guix_event_queue.free_end = guix_event_queue.first;
    guix_event_queue.first = guix_event_queue.first -> next;
    guix_event_queue.free_end -> next = GX_NULL;

    if (!guix_event_queue.first) {
        guix_event_queue.last = GX_NULL;
    }

    /* check for popping out a timer event */
    if (put_event -> gx_event_type == GX_EVENT_TIMER) {
        guix_timer_event_pending = GX_FALSE;
    }

    /* unlock the queue */
    os_mutex_post(&guix_event_queue.lock);
    return GX_SUCCESS;
}

/* event_purge: delete events targetted to particular widget */
VOID   gx_generic_event_purge(GX_WIDGET *target)
{
    GX_BOOL Purge;
    GUIX_LINKED_EVENT *pTest;

    /* Lock down the guix queue */

    os_mutex_pend(&guix_event_queue.lock, 0);

    // look for events targetted to widget or widget children:
    pTest = guix_event_queue.first;

    while (pTest) {
        Purge = GX_FALSE;

        if (pTest ->event_data.gx_event_target) {
            if (pTest -> event_data.gx_event_target == target) {
                Purge = GX_TRUE;
            } else {
                gx_widget_child_detect(target, pTest->event_data.gx_event_target, &Purge);
            }
            if (Purge) {
                pTest ->event_data.gx_event_target = GX_NULL;
                pTest->event_data.gx_event_type = 0;
            }
        }
        pTest = pTest -> next;
    }

    os_mutex_post(&guix_event_queue.lock);
}

/* start the RTOS timer */
VOID   gx_generic_timer_start(VOID)
{
    puts("gx_generic_timer_start.\r\n");

    guix_timer_task_run_flag = 1;
    os_sem_set(&guix_timer_task_run_sem, 0);
    os_sem_post(&guix_timer_task_run_sem);
    /*os_task_resume("guix_timer_task");*/
}

/* stop the RTOS timer */
VOID   gx_generic_timer_stop(VOID)
{
    puts("gx_generic_timer_stop.\r\n");

    os_sem_set(&guix_timer_task_run_sem, 0);
    guix_timer_task_run_flag = 0;
    /*os_task_suspend("guix_timer_task");*/
}

/* lock the system protection mutex */
VOID   gx_generic_system_mutex_lock(VOID)
{
    os_mutex_pend(&guix_system_lock_mutex, 0);
}

/* unlock system protection mutex */
VOID   gx_generic_system_mutex_unlock(VOID)
{
    os_mutex_post(&guix_system_lock_mutex);
}

/* return number of low-level system timer ticks. Used for pen speed caculations */
ULONG  gx_generic_system_time_get(VOID)
{
    return ((ULONG)os_time_get());
}

/* thread_identify: return current thread identifier, cast as VOID * */
VOID  *gx_generic_thread_identify(VOID)
{
    return ((VOID *)get_cur_thread_pid());
}

/* a simple task to drive the GUIX timer mechanism */
void guix_timer_task_entry(void *unused)
{
    while (1) {
        if (!guix_timer_task_run_flag) {
            os_sem_pend(&guix_timer_task_run_sem, 0)	;
        }

        /* prevent sending timer events faster than they can be processed */
        if (!guix_timer_event_pending) {
            guix_timer_event_pending = GX_TRUE;
            _gx_system_timer_expiration(0);
        }

        msleep(20);
    }
}

VOID gx_generic_time_delay(int ticks)
{
    os_time_dly(ticks);
}

#endif



#if defined(GUIX_BINDING_UCOS_II)
#endif

#endif  /* if !ThreadX */


