//
// Created by chris on 5/15/2016.
//

#include "fthread.h"

typedef struct thread* thread_type;
typedef struct mutex_lock* mutex_lock_type;
typedef struct cond_var* cond_var_type;
typedef void* (*top_level_procedure)(va_list args);
typedef void* fifoq_p;
struct mutex_lock {
    thread_type holder;
    fifoq_p waiting;
};
/**
 * This creates a new thread that starts execution in the top-level procedure,
 * with the supplied args as actual parameters for the formal specified in the
 * procedure prototype.
 *
 * @param child function pointer to be run on the new thread
 * @param ... arguments to be passed into procedure
 *
 * @return the new thread
 */
thread_type thread_create(top_level_procedure child, .../*args*/);

/**
 * This terminates the thread given by the tid.
 *
 * @param tid thread to terminate
 */
void thread_terminate(thread_type tid);

/**
 * When the thread returns it has mylock; the calling thread blocks if the lock
 * is currently by some other thread.
 *
 * @param mylock lock requested
 */
void thread_mutex_lock(mutex_lock_type mylock);

/**
 * If the thread currently has mylock it is released; error results otherwise.
 *
 * @return 0 if execution success 1 if mylock was already unlocked
 */
int thread_mutex_unlock(mutex_lock_type mylock);

/**
 * The call does not block the calling thread; instead it returns true if the
 * thread gets mylock or false if it is currently in use by some other thread.
 *
 * @param mylock lock to try
 */
bool thread_mutex_trylock(mutex_lock_type mylock);

/**
 * The calling thread blocks until the thread given by peer_thread_id
 * terminates.
 *
 * @param peer_thread_id thread to wait for
 */
void thread_join(thread_type peer_thread_id);

/**
 * The calling thread blocks the condition variable buf_not_empty; the library
 * implicitly releases the lock buflock; error results if the lock is not
 * currently held by the calling thread.
 *
 * @param buf_not_empty
 * @param buflock
 *
 * @return 0 if execution success 1 if the lock is not currently held by the
 *           calling thread.
 */
int thread_cond_wait(cond_var_type buf_not_empty, mutex_lock_type buflock);

/**
 * A thread (if any) waiting on the condition variable buf_not_empty is woken
 * up; the awakened thread is ready for execution if the lock associated with
 * it (in the wait call) is currently available; if not, the thread is moved
 * from the queue for the condition variable to the appropriate lock queue.
 *
 * @
 */
void thread_cond_signal(cond_var_type buf_not_empty);
