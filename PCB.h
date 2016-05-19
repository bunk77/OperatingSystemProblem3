/*
 * Problem 3 - Discontinuities
 * TCSS 422 A Spring 2016
 * Bun Kak, Chris Ottersen, Mark Peters, Paul Zander
 */

#ifndef PCB_H
#define PCB_H

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <limits.h>

/* DOESN'T DISPLAY WARNINGS THAT ARE DUE TO ULONG WORD CAST */
//words are ulongs, silly
//#pragma GCC diagnostic ignored "-Wformat="
#define created created_
#define bool bool_
#define true true_
#define false false_

#define REGNUM (REG_COUNT + IO_NUMBER*IO_CALLS)
#define PRIORITIES_TOTAL 16
#define LOWEST_PRIORITY (PRIORITIES_TOTAL - 1)
#define IO_NUMBER 2
#define IO_CALLS 4
#define REG_COUNT 5

#define MAX_PC_MIN 50
#define MAX_PC_RANGE 3000
#define MIN_IO_CALL 25
#define TERM_RANGE 15
#define TERM_INFINITE_CHANCE 10

#define DEFAULT_STATE created
#define DEFAULT_PC 0Lu
#define DEFAULT_SW 0Lu

#define PCB_NULL_ERROR 5
#define PCB_INIT_ERROR 7
#define PCB_OTHER_ERROR 41

#define PCB_TOSTRING_LEN 180

typedef enum {false, true} bool;
enum state_type {created = 0, ready, running, interrupted, waiting, terminated};


//typedef struct pcb PCB;
typedef unsigned long word;
typedef struct PCB * PCB_p;
typedef struct CPU * CPU_p;
typedef union regfile REG;
typedef REG * REG_p;

struct PCB {
  REG_p regs;
  //separate from sysStack--don't push/pop  
  word pid;        // process ID #, a unique number
  unsigned short priority;  // priorities 0=highest, LOWEST_PRIORITY=lowest
  enum state_type state;    // process state (running, waiting, etc.)
  word timeCreate;
  word timeTerminate;
  
};

union regfile {
            //save to PCB from system
    struct {
        word pc, MAX_PC, sw, term_count, TERMINATE, IO_TRAPS[IO_NUMBER][IO_CALLS];
    } reg;
    word gpu[REGNUM];
};

PCB_p 			PCB_construct 	(int *ptr_error);
PCB_p 			PCB_construct_init 	(int *ptr_error);
int 			PCB_destruct 	(PCB_p this); 
REG_p REG_init(REG_p this, int *ptr_error);
int 			PCB_init 		(PCB_p this);      
int 			PCB_setPid 		(PCB_p this, word pid); 
word 	PCB_getPid 		(PCB_p this, int *ptr_error);
int 			PCB_setState 	(PCB_p this, enum state_type state); 
enum state_type PCB_getState 	(PCB_p this, int *ptr_error);
int 			PCB_setPriority (PCB_p this, unsigned short priority); 
unsigned short 	PCB_getPriority (PCB_p this, int *ptr_error);
int 			PCB_setPc 		(PCB_p this, word pc); 
word 	PCB_getPc 		(PCB_p this, int *ptr_error);
int 			PCB_setSw 		(PCB_p this, word sw); 
word 	PCB_getSw 		(PCB_p this, int *ptr_error);
char* 			PCB_toString 	(PCB_p this, char *str, int *ptr_error);
int 			PCB_compareTo	(PCB_p this, PCB_p other, int *ptr_error);

int 			PCB_test_main	(int argc, char** argv);
char * Reg_File_toString(REG_p this, char *str, int *ptr_error);

#endif

