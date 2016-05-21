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
#include "priorityq.h"
#include "pcb.h"
typedef struct mutex *mutex_p;
typedef struct condition *condition_p;

struct mutex {
  PCB_p owner;
  FIFOQp waiting_queue;
};

struct condition {
  PCB_p owner;
  FIFOQp waiting_queue;
};

struct process_lock_relation {

};
void fwait(mutex_p mutex, uint64_t period);
// t=current time+period
// while(current time < period);

void flock(condition_p cond, PCB_p pcb);
//
void funlock(condition_p cond);
void fsignal(condition_p cond);
bool ftrylock(condition_p cond);
/*
-----------------------------------------------
fthread.c
-----------------------------------------------

Mutex malloc
PCB pointer to current owner of lock
    fifoQ of processes waiting for the lock


Condition malloc
fifoQ of processes waiting for signal
    parallel fifoQ of the mutex for each process


lock()
unlock()
mutex then must put head of queue into readyQ
    wait()
signal()
cond puts head of queue into its mutex
trylock()*/
#endif //THREADS_FTHREAD_H
