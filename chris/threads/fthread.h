//
// Created by chris on 5/15/2016.
//

#ifndef THREADS_FTHREAD_H
#define THREADS_FTHREAD_H


/*
 * Mutex malloc
 * PCB pointer to current owner of lock
 * fifoQ of processes waiting for the lock
 *
 *
 * Condition malloc
 * fifoQ of processes waiting for signal
 * parallel fifoQ of the mutex for each process
 *
 *
 * lock()
 * unlock()
 *     mutex then must put head of queue into readyQ
 * wait()
 * signal()
 *     cond puts head of queue into its mutex
 * trylock()
 */
#include <stdarg.h>
#include <stdbool.h>
#include "threadq.h"
#include "pcb.h"


/**
 * Mutex lock type. This data structure is simply a wrapper for a
 * {@code THREADQp} object.
 */
typedef struct mutex_lock* mutex_lock_type;

/**
 * Mutex lock type. This data structure is simply a wrapper for a
 * {@code THREADQp} object. Note, this queue is actually comprised of
 * {@code thread_condition_t} objects, since THREADQp does not actually modify
 * it's elements, it is possible (and easier) to just cast the
 * {@code thread_condition_t}'s to {@code thread_type}.
 */
typedef struct cond_var* cond_var_type;

typedef void* thread_arguments;
typedef void* (*top_level_procedure)(thread_arguments args);

/**
 * Thread condition type. This data structure associates a single thread with a
 * single mutex (for use in signaling).
 */
typedef struct thread_condition *thread_condition_t;

//
// Created by chris on 5/15/2016.
//
#include <stdlib.h>
#include "fthread.h"

#define NO_ERRORS 0
#define NULL_POINTER 1
#define MUTEX_WAITING_QUEUE_NOT_EMPTY 2
#define COND_WAITING_QUEUE_NOT_EMPTY 4

/**
 * This terminates the thread given by the tid.
 *
 * @param tid thread to terminate
 * @param ptr_error pointer to hold any errors
 */
void thread_terminate(thread_type tid, uint64_t *ptr_error);

/**
 * When the thread returns it has {@code mylock}; the calling thread blocks if
 * the lock is currently by some other thread.
 *
 * @param tid thread object which is requesting the lock.
 * @param mylock lock requested.
 * @param ptr_error pointer to hold any errors.
 * @return {@code true} if a lock was aquired, {@code false} otherwise.
 */
bool thread_mutex_lock(thread_type tid, mutex_lock_type mylock,
                       uint64_t *ptr_error);

/**
 * If the thread currently has {@code mylock} it is released; error results
 * otherwise.
 *
 * @param tid thread object which is relinquishing the lock.
 * @param mylock mutex holding the lock.
 * @param ptr_error pointer to hold any errors.
 * @return the thread now in posession of {@code mylock}, {@code NULL}
 *          otherwise.
 */
thread_type thread_mutex_unlock(thread_type tid, mutex_lock_type mylock,
                                uint64_t *ptr_error);

/**
 * The call does not block the calling thread; instead it returns true if the
 * thread gets mylock or false if it is currently in use by some other thread.
 *
 * @param tid thread object which is requesting the lock.
 * @param mylock lock to attempt to acquire.
 * @param ptr_error pointer to hold any errors.
 * @return {@code true} if a lock was acquired, {@code false} otherwise.
 */
bool thread_mutex_trylock(thread_type tid, mutex_lock_type mylock,
                          uint64_t *ptr_error);

/**
 * The calling thread ({@code tid}) blocks until the thread given by
 * {@code peer_thread_id}. terminates.
 *
 * @param tid thread asking to wait.
 * @param peer_thread_id thread to wait for.
 * @param ptr_error pointer to hold any errors.
 */
void thread_join(thread_type tid, thread_type peer_thread_id,
                 uint64_t *ptr_error);

/**
 * The calling thread ({@code tid}) blocks the condition variable
 * {@code buf_not_empty}; the library implicitly releases the lock
 * {@code buflock}; error results if the lock is not currently held by the
 * calling thread.
 *
 * @param tid thread asking to wait.
 * @param buf_not_empty thread condition variable indicating a thread and the
 *          mutex it is waiting for.
 * @param buflock mutex to be locked.
 * @param ptr_error pointer to hold any errors.
 *
 * @return {@code false} if execution success {@code true} if the lock is not
 *           currently held by the calling thread.
 */
bool thread_cond_wait(thread_type tid, cond_var_type buf_not_empty,
                      mutex_lock_type buflock, uint64_t *ptr_error);

/**
 * A thread (if any) waiting on the condition variable {@code buf_not_empty} is
 * woken up; the awakened thread is ready for execution if the lock associated
 * with it (in the wait call) is currently available; if not, the thread is
 * moved from the queue for the condition variable to the appropriate lock
 * queue.
 *
 * @param buf_not_empty condition, having been met, to be signaled.
 * @param ptr_error pointer to hold any errors.
 */
void thread_cond_signal(cond_var_type buf_not_empty, uint64_t *ptr_error);

/**
 * Create a new {@code mutex_lock_type} variable.
 *
 * @param ptr_error pointer to hold any errors.
 * @return newly created lock.
 */
mutex_lock_type mutex_lock_create(uint64_t *ptr_error);

/**
 * Clean out the mutex's thread queue.
 *
 * @param this the mutex to be terminated.
 * @param ptr_error pointer to hold any errors.
 */
void mutex_lock_terminate(mutex_lock_type this, uint64_t *ptr_error);


/**
 * Create a new {@code cond_var_type} variable.
 *
 * @param ptr_error pointer to hold any errors.
 * @return newly created condition variable.
 */
cond_var_type cond_var_create(uint64_t *ptr_error);

/**
 * Clean out the condition variable's thread queue.
 *
 * @param this the condition variable to be terminated.
 * @param ptr_error pointer to hold any errors.
 */
void cond_var_terminate(cond_var_type this, uint64_t *ptr_error);
#endif //THREADS_FTHREAD_H
