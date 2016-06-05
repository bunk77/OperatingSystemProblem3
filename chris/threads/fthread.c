//
// Created by chris on 5/15/2016.
//
#include <stdlib.h>
#include "fthread.h"

typedef struct thread_condition *thread_condition_t;

struct mutex_lock {
    THREADQp waiting;
};

struct thread {
    PCB_p pcb;

    top_level_procedure procedure;
    bool terminate;
    thread_arguments args;
    //enum state_type status;
    THREADQp buddies;
};

struct thread_condition {
    thread_type tid;
    mutex_lock_type mutex;
};

struct cond_var {
    THREADQp waiting;
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
    this->buddies = THREADQ_construct(NULL);//TODO:FIFOQ_construct
    this->procedure = child;
    this->args = args;
    this->terminate = false;
    //this->status = new;
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
    while (!THREADQ_is_empty(tid->buddies, NULL)) {//TODO:FIFOQ_size
        thread_type buddy = THREADQ_dequeue(tid->buddies, false,
                                            NULL);//TODO:FIFOQ_dequeue
    }
    THREADQ_destruct(tid->buddies, NULL);//TODO:FIFOQ_destruct
    free(tid);
}

/**
 * When the thread returns it has mylock; the calling thread blocks if the lock
 * is currently by some other thread.
 *
 * @param mylock lock requested
 */
bool thread_mutex_lock(thread_type tid, mutex_lock_type mylock)
{

    if (!THREADQ_is_empty(mylock->waiting, NULL)
        || THREADQ_peek(mylock->waiting, NULL) ==
           tid) {//TODO:FIFOQ_size, FIFOQ_peek
        THREADQ_enqueue(mylock->waiting, tid, NULL);//TODO:FIFOQ_enqueue
        return false;
    }
    return true;

}

/**
 * If the thread currently has mylock it is released; error results otherwise.
 *
 * @return 0 if execution success 1 if mylock was already unlocked
 */
void *thread_mutex_unlock(thread_type tid, mutex_lock_type mylock)
{

    if (THREADQ_peek(mylock->waiting, NULL) == tid) {//TODO:FIFOQ_peek

        THREADQ_dequeue(mylock->waiting, false, NULL);//TODO:FIFOQ_dequeue
//        return 0;
//    } else {
//        return 1;
    }

    return ((thread_type) THREADQ_peek(mylock->waiting,
                                       NULL))->pcb;//TODO:FIFOQ_peek
}

/**
 * The call does not block the calling thread; instead it returns true if the
 * thread gets mylock or false if it is currently in use by some other thread.
 *
 * @param mylock lock to try
 */
bool thread_mutex_trylock(thread_type tid, mutex_lock_type mylock) {

    return THREADQ_peek(mylock->waiting, NULL) != tid;//TODO:FIFOQ_peek
}

/**
 * The calling thread blocks until the thread given by peer_thread_id
 * terminates.
 *
 * @param peer_thread_id thread to wait for
 */
void thread_join(thread_type tid, thread_type peer_thread_id) {
    if (!peer_thread_id->terminate) {
        THREADQ_enqueue(peer_thread_id->buddies, tid, NULL);//TODO:FIFOQ_enqueue
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

    if (THREADQ_peek(buflock->waiting, NULL) == tid) {//TODO:FIFOQ_peek
        thread_condition_t cond = malloc(sizeof(struct thread_condition));

        cond->tid = THREADQ_dequeue(buflock->waiting, false,
                                    NULL);//TODO:FIFOQ_dequeue
        cond->mutex = buflock;

        THREADQ_enqueue(buf_not_empty->waiting, cond, NULL);//TODO:FIFOQ_enqueue
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
void *thread_cond_signal(cond_var_type buf_not_empty)
{
    thread_condition_t cond = NULL;

    while (!THREADQ_is_empty(buf_not_empty->waiting, NULL)) {//TODO:FIFOQ_size

        cond = THREADQ_dequeue(buf_not_empty->waiting, false,
                               NULL);//TODO:FIFOQ_dequeue
        free(cond);
    }
    return cond;

}

//todo:group->mutex[g] = create mutex function
mutex_lock_type mutex_lock_create(uint64_t *ptr_error)
{
    mutex_lock_type this = malloc(sizeof(struct mutex_lock));
    this->waiting = THREADQ_construct(ptr_error);

    return this;
}

//todo:group->mutex[g] = destroy mutex (error if waiting queue is empty)
void mutex_lock_terminate(mutex_lock_type this, uint64_t *ptr_error)
{
    this->waiting = THREADQ_construct(ptr_error);
    if (THREADQ_is_empty(this->waiting, ptr_error)) {
        THREADQ_destruct(this->waiting, ptr_error);
        free(this);
    } else {
        // todo:add error
    }
}

//todo:group->cond[g] = destroy condition (error if waiting queue is empty)
cond_var_type cond_var_create(uint64_t *ptr_error)
{
    cond_var_type this = malloc(sizeof(struct cond_var));
    this->waiting = THREADQ_construct(ptr_error);
    return this;
}


//todo:group->cond[g] = create condition function
cond_var_type cond_var_terminate(cond_var_type this, uint64_t *ptr_error)
{
    this->waiting = THREADQ_construct(ptr_error);
    if (THREADQ_is_empty(this->waiting, ptr_error)) {
        THREADQ_destruct(this->waiting, ptr_error);
        free(this);
    } else {
        // todo:add error
    }
}