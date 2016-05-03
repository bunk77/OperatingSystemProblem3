/*
 * Problem 3 - Discontinuities
 * TCSS 422 A Spring 2016
 * Bun Kak, Chris Ottersen, Mark Peters, Paul Zander
 */

#ifndef OS_H
#define OS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <pthread.h>
//#include "lib.h"
#include "FIFOq.h"
#include "PCB.h"

#define thread pthread_t
#define mutex pthread_mutex_t
#define cond pthread_cond_t

#define DEBUG false
#define EXIT_STATUS_MESSAGE true
#define OUTPUT true
#define START_IDLE false
#define NO_INTERRUPT 9999
#define INTERRUPT_TIMER 5555
#define INTERRUPT_TERMINATE 8888
#define INTERRUPT_IOCOMPLETE 4444

#define TIME_QUANTUM 300
#define MAX_PROCESSES 30
#define SYSSIZE 256
#define STACK_ERROR_DEFAULT 0
#define MAX_NEW_PCB 5
#define RUN_MIN_TIME 3000
#define RUN_TIME_RANGE 1000

#define CPU_NULL_ERROR 71
#define CPU_STACK_ERROR 73

//note on style: ALLCAPS_lowercase is either a MUTEX_ variable or the data
//               protected by the MUTEX_

struct io_thread_type {
    thread THREAD_io;
    mutex MUTEX_io;
    bool INTERRUPT_iocomplete;
    FIFOq_p waitingQ;

    /*struct {
        bool interrupt_io_complete;
        FIFOq_p waitingQ_IO;
    } waitingIO[ioThreadCount];*/
};

typedef struct io_thread_type* io_thread;

//extern word SsyStack[SYSSIZE];
//extern int SysPointer;

int      bootOS          ();
int      mainLoopOS      (int *error);
void*    timer           (void*);
void*    io              (void*);
void     trap_terminate  ();
void     trap_iohandler  (const int T, int* error);
void     interrupt       (const int INTERRUPT, void*, int* error);
void     isr_timer       (int* error);
void     isr_iocomplete  (const int IO, int* error);
void     scheduler       (const int INTERRUPT, int* error);
void     dispatcher      (int* error);
int      createPCBs  	 (int *error);
int      sysStackPush    (REG_p, int* error);
int      sysStackPop     (REG_p, int* error);
void     cleanup         (int* error);
void     queueCleanup    (FIFOq_p, char*, int* error);
void     stackCleanup    ();
//static void     run             (word *pc, int *error);

#endif
