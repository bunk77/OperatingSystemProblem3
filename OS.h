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
#include "FIFOq.h"
#include "PCB.h"

/* DOESN'T DISPLAY WARNINGS THAT ARE DUE TO ULONG WORD CAST */
//words are ulongs, silly
#pragma GCC diagnostic ignored "-Wformat="

#define thread pthread_t
#define mutex pthread_mutex_t
#define cond pthread_cond_t

#define DEBUG true
#define STACK_DEBUG false
#define EXIT_STATUS_MESSAGE true
#define OUTPUT true
#define OUTPUT_CONTEXT_SWITCH 1

#define START_IDLE false
#define PCB_CREATE_EVERY false
#define PCB_CREATE_FIRST false
#define PCB_SCHEDULE_EVERY false

#define NO_INTERRUPT 9999
#define INTERRUPT_TIMER 5555
#define INTERRUPT_TERMINATE 8888
#define INTERRUPT_IOCOMPLETE 4444

#define TIME_QUANTUM 300
#define TIMER_SLEEP (TIME_QUANTUM * 100000)
#define MAX_PROCESSES 30
#define SYSSIZE 256
#define STACK_ERROR_DEFAULT 0
#define MAX_NEW_PCB 5
#define RUN_MIN_TIME 3000
#define RUN_TIME_RANGE 1000

#define OS_NO_ERROR EXIT_SUCCESS
#define CPU_NULL_ERROR 71
#define CPU_STACK_ERROR 73


//note on style: ALLCAPS_lowercase is either a MUTEX_ variable or the data
//               protected by the MUTEX_

struct CPU {
  REG_p regs;
};

struct io_thread_type {
    thread THREAD_io;
    mutex MUTEX_io;
    cond COND_io;
    bool INTERRUPT_iocomplete;
    bool SHUTOFF_io;
    FIFOq_p waitingQ;
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
void     scheduler       (int* error);
void     dispatcher      (int* error);
int      createPCBs  	 (int *error);
int      sysStackPush    (REG_p, int* error);
int      sysStackPop     (REG_p, int* error);
void     cleanup         (int* error);
void     queueCleanup    (FIFOq_p, char*, int* error);
void     stackCleanup    ();
void     nanosleeptest   ();
//static void     run             (word *pc, int *error);

#endif
