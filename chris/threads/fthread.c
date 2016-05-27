//
// Created by chris on 5/15/2016.
//
#include <stdlib.h>
#include "fthread.h"
#include "fifoq.h"
//enum stat {created, ready, running, interrupted, blocked, terminated};
typedef struct thread_condition *thread_condition_type;

struct mutex_lock {
    FIFOQp waiting;
};

struct thread {
    PCB_p pcb;
    top_level_procedure procedure;
    bool terminate;
    thread_arguments args;
    enum state_type status;
    FIFOQp buddies;
};
struct thread_condition {
    thread_type tid;
    mutex_lock_type mutex;
};

struct cond_var {
    FIFOQp waiting;
};


/**
 * This creates a new thread that starts execution in the top-level procedure,
 * with the supplied args as actual parameters for the formal specified in the
 * procedure prototype.
 *
 * @param child function pointer to be run on the new thread
 * @param args to be passed into procedure
 *
 * @return the new thread
 */
thread_type thread_create(top_level_procedure child, thread_arguments args) {
    thread_type this = malloc(sizeof(struct thread));
    this->buddies = FIFOQ_construct(NULL);
    this->procedure = child;
    this->args = args;
    this->terminate = false;
    this->status = new;
    this->procedure(args);
    return this;
    //this->args = va
}

/**
 * This terminates the thread given by the tid.
 *
 * @param tid thread to terminate
 */
void thread_terminate(thread_type tid) {
    tid->terminate = true;
    while(FIFOQ_size(tid->buddies) != 0) {
        thread_type buddy = FIFOQ_dequeue(tid->buddies);
        //PCB_setStatus(buddy->pcb->status, ready);
    }
    FIFOQ_destruct(tid->buddies);
    free(tid);
}

/**
 * When the thread returns it has mylock; the calling thread blocks if the lock
 * is currently by some other thread.
 *
 * @param mylock lock requested
 */
void thread_mutex_lock(thread_type tid, mutex_lock_type mylock) {
    if (FIFOQ_size(mylock->waiting) != 0 || !FIFOQ_peek(mylock->waiting) == tid ) {
        FIFOQ_enqueue(mylock->waiting, tid);
    }
}

/**
 * If the thread currently has mylock it is released; error results otherwise.
 *
 * @return 0 if execution success 1 if mylock was already unlocked
 */
int thread_mutex_unlock(thread_type tid, mutex_lock_type mylock) {
    if(FIFOQ_peek(mylock->waiting) == tid) {
        FIFOQ_dequeue(mylock->waiting);
        return 0;
    } else {
        return 1;
    }
}

/**
 * The call does not block the calling thread; instead it returns true if the
 * thread gets mylock or false if it is currently in use by some other thread.
 *
 * @param mylock lock to try
 */
bool thread_mutex_trylock(thread_type tid, mutex_lock_type mylock) {
    return FIFOQ_peek(mylock->waiting) != tid;
}

/**
 * The calling thread blocks until the thread given by peer_thread_id
 * terminates.
 *
 * @param peer_thread_id thread to wait for
 */
void thread_join(thread_type tid, thread_type peer_thread_id) {
    if (!peer_thread_id->terminate) {
        FIFOQ_enqueue(peer_thread_id->buddies, tid);
    }
}

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
uint64_t thread_cond_wait(thread_type tid, cond_var_type buf_not_empty, mutex_lock_type buflock) {
    // does tid hold buflock?
    if (FIFOQ_peek(buflock->waiting) == tid) {
        thread_condition_type cond = malloc(sizeof(struct thread_condition));
        cond->tid = FIFOQ_dequeue(buflock->waiting);
        cond->mutex = buflock;
        FIFOQ_enqueue(buf_not_empty->waiting, cond);
        return 0;
    } else {
        return 1;
    }
}

/**
 * A thread (if any) waiting on the condition variable buf_not_empty is woken
 * up; the awakened thread is ready for execution if the lock associated with
 * it (in the wait call) is currently available; if not, the thread is moved
 * from the queue for the condition variable to the appropriate lock queue.
 *
 * @param buf_not_empty
 */
void thread_cond_signal(cond_var_type buf_not_empty) {
    while (FIFOQ_size(buf_not_empty->waiting) > 0) {
        thread_condition_type cond = FIFOQ_dequeue(buf_not_empty->waiting);
        free(cond);
    }
    //free(buf_not_empty);
}
