//
// Created by chris on 5/15/2016.
//
#include <stdlib.h>
#include "fthread.h"

#define NO_ERRORS 0
#define NULL_POINTER 1
#define MUTEX_WAITING_QUEUE_NOT_EMPTY 2
#define COND_WAITING_QUEUE_NOT_EMPTY 4
#define THREAD_DOES_NOT_OWN_MUTEX 8
#define THREAD_DOES_NOT_OWN_LOCK 16

/**
 * Mutex lock type. This data structure is simply a wrapper for a
 * {@code THREADQp} object.
 */
struct mutex_lock {
    /**
     * Queue of threads waiting for a resource.
     */
    THREADQp waiting;
};

/**
 * Thread condition type. This data structure associates a single thread with a
 * single mutex (for use in signaling).
 */
struct thread_condition {
    /** thread */
    thread_type tid;
    /** mutex of which {@code tid} is a member */
    mutex_lock_type mutex;
};

/**
 * Mutex lock type. This data structure is simply a wrapper for a
 * {@code THREADQp} object. Note, this queue is actually comprised of
 * {@code thread_condition_t} objects, since THREADQp does not actually modify
 * it's elements, it is possible (and easier) to just cast the
 * {@code thread_condition_t}'s to {@code thread_type}.
 */
struct cond_var {
    /**
     * Queue of {@code thread_condition}s waiting for a a signal.
     */
    THREADQp waiting;
};


/**
 * This terminates the thread given by the tid.
 *
 * @param tid thread to terminate
 * @param ptr_error pointer to hold any errors
 */
void thread_terminate(thread_type tid, uint64_t *ptr_error)
{
    tid->terminate = true;
    while (!THREADQ_is_empty(tid->buddies, ptr_error)) {
        thread_type buddy = THREADQ_dequeue(tid->buddies, ptr_error);
        buddy->state = ready;
    }
    THREADQ_destruct(tid->buddies, ptr_error);
    //free(tid);
}

/**
 * When the thread returns it has {@code mylock}; the calling thread blocks if
 * the lock is currently by some other thread.
 *
 * @param tid thread object which is requesting the lock.
 * @param mylock lock requested.
 * @param ptr_error pointer to hold any errors.
 * @return {@code true} if a lock was acquired, {@code false} otherwise.
 */
bool thread_mutex_lock(thread_type tid, mutex_lock_type mylock,
                       uint64_t *ptr_error)
{
    if (THREADQ_is_empty(mylock->waiting, ptr_error)) {
        THREADQ_enqueue(mylock->waiting, tid, ptr_error);
        //tid->state = ready;
        return true;
    } else if (THREADQ_peek(mylock->waiting, ptr_error) == tid) {
        //tid->state = blocked;
        return true;
    }
    return false;
}

/**
 * If the thread currently has {@code mylock} it is released; error results
 * otherwise.
 *
 * @param tid thread object which is relinquishing the lock.
 * @param mylock mutex holding the lock.
 * @param ptr_error pointer to hold any errors.
 * @return the thread now in possession of {@code mylock}, {@code NULL}
 *          otherwise.
 */
thread_type thread_mutex_unlock(thread_type tid, mutex_lock_type mylock,
                                uint64_t *ptr_error)
{

    if (THREADQ_peek(mylock->waiting, ptr_error) == tid) {
        THREADQ_dequeue(mylock->waiting, ptr_error);
        return THREADQ_peek(mylock->waiting, ptr_error);
    } else {
        *ptr_error += THREAD_DOES_NOT_OWN_MUTEX;
        return NULL;
    }
}

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
                          uint64_t *ptr_error)
{
    if (THREADQ_is_empty(mylock->waiting, ptr_error)) {
        THREADQ_enqueue(mylock->waiting, tid, ptr_error);
        return true;
    } else {
        return THREADQ_peek(mylock->waiting, ptr_error) == tid;
    }
    return false;
}

/**
 * The calling thread ({@code tid}) blocks until the thread given by
 * {@code peer_thread_id} terminates.
 *
 * @param tid thread asking to wait.
 * @param peer_thread_id thread to wait for.
 * @param ptr_error pointer to hold any errors.
 */
void thread_join(thread_type tid, thread_type peer_thread_id,
                 uint64_t *ptr_error)
{
    if (!peer_thread_id->terminate) {
        THREADQ_enqueue(peer_thread_id->buddies, tid, ptr_error);
    }
}

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
                      mutex_lock_type buflock, uint64_t *ptr_error)
{
    if (THREADQ_peek(buflock->waiting, NULL) == tid) {
        thread_condition_t cond = malloc(sizeof(struct thread_condition));

        cond->tid = THREADQ_dequeue(buflock->waiting, ptr_error);
        cond->mutex = buflock;

        THREADQ_enqueue(buf_not_empty->waiting, cond, ptr_error);
        return false;
    } else {
        *ptr_error += THREAD_DOES_NOT_OWN_LOCK;
        return true;
    }
}

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
void thread_cond_signal(cond_var_type buf_not_empty, uint64_t *ptr_error)
{
    while (!THREADQ_is_empty(buf_not_empty->waiting, ptr_error)) {
        thread_condition_t cond = (thread_condition_t)
            THREADQ_dequeue(buf_not_empty->waiting, ptr_error);
        free(cond);
    }
}

/**
 * Create a new {@code mutex_lock_type} variable.
 *
 * @param ptr_error pointer to hold any errors.
 * @return newly created lock.
 */
mutex_lock_type mutex_lock_create(uint64_t *ptr_error)
{
    mutex_lock_type this = malloc(sizeof(struct mutex_lock));
    this->waiting = THREADQ_construct(ptr_error);
    return this;
}

/**
 * Clean out the mutex's thread queue.
 *
 * @param this the mutex to be terminated.
 * @param ptr_error pointer to hold any errors.
 */
void mutex_lock_terminate(mutex_lock_type this, uint64_t *ptr_error)
{
    if (THREADQ_is_empty(this->waiting, ptr_error)) {
        THREADQ_destruct(this->waiting, ptr_error);
        free(this);
    } else if (ptr_error != NULL) {
        *ptr_error += MUTEX_WAITING_QUEUE_NOT_EMPTY;
    }
}


/**
 * Create a new {@code cond_var_type} variable.
 *
 * @param ptr_error pointer to hold any errors.
 * @return newly created condition variable.
 */
cond_var_type cond_var_create(uint64_t *ptr_error)
{
    cond_var_type this = malloc(sizeof(struct cond_var));
    this->waiting = THREADQ_construct(ptr_error);
    return this;
}

/**
 * Clean out the condition variable's thread queue.
 *
 * @param this the condition variable to be terminated.
 * @param ptr_error pointer to hold any errors.
 */
void cond_var_terminate(cond_var_type this, uint64_t *ptr_error)
{
    if (THREADQ_is_empty(this->waiting, ptr_error)) {
        THREADQ_destruct(this->waiting, ptr_error);
        free(this);
    } else {
        *ptr_error += COND_WAITING_QUEUE_NOT_EMPTY;
    }
}