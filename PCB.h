/*
 * Problem 3 - Discontinuities
 * TCSS 422 A Spring 2016
 * Bun Kak, Chris Ottersen, Mark Peters, Paul Zander
 */

#ifndef PCB_H
#define PCB_H
#define created created_
#define bool bool_
#define true true_
#define false false_

#define NUMREGS 16
#define PRIORITIES_TOTAL 16
#define LOWEST_PRIORITY (PRIORITIES_TOTAL - 1)
#define IO_NUMBER 2
#define IO_CALLS 4

#define MAX_PC_MIN 100
#define MAX_PC_RANGE 2000
#define TERM_RANGE 10

#define DEFAULT_STATE created
#define DEFAULT_PC 0Lu
#define DEFAULT_SW 0Lu

#define PCB_NULL_ERROR 5
#define PCB_INIT_ERROR 7
#define PCB_OTHER_ERROR 41

#define PCB_TOSTRING_LEN 80

typedef enum {false, true} bool;
enum state_type {created = 0, ready, running, interrupted, waiting, terminated};


struct CPU {
  REG_p regs;
};

struct PCB {
  REG_p regs;
  //separate from sysStack--don't push/pop  
  unsigned long pid;        // process ID #, a unique number
  unsigned short priority;  // priorities 0=highest, LOWEST_PRIORITY=lowest
  enum state_type state;    // process state (running, waiting, etc.)
  unsigned long timeCreate;
  unsigned long timeTerminate;
  
};

struct regfile {
  //save to PCB from system
  unsigned long pc;         // holds the current pc value when pre emptied
  unsigned long MAX_PC;
  unsigned long sw;         // sw register for sw stuff
  unsigned long term_count;
  unsigned long TERMINATE;
  unsigned long IO_TRAPS[IO_NUMBER][IO_CALLS];
    
};


//typedef struct pcb PCB;
typedef struct PCB * PCB_p;
typedef struct CPU * CPU_p;
typedef struct regfile * REG_p;


PCB_p 			PCB_construct 	(int *ptr_error);
PCB_p 			PCB_construct_init 	(int *ptr_error);
int 			PCB_destruct 	(PCB_p this); 
REG_p REG_init(REG_p this, int *ptr_error);
int 			PCB_init 		(PCB_p this);      
int 			PCB_setPid 		(PCB_p this, unsigned long pid); 
unsigned long 	PCB_getPid 		(PCB_p this, int *ptr_error);
int 			PCB_setState 	(PCB_p this, enum state_type state); 
enum state_type PCB_getState 	(PCB_p this, int *ptr_error);
int 			PCB_setPriority (PCB_p this, unsigned short priority); 
unsigned short 	PCB_getPriority (PCB_p this, int *ptr_error);
int 			PCB_setPc 		(PCB_p this, unsigned long pc); 
unsigned long 	PCB_getPc 		(PCB_p this, int *ptr_error);
int 			PCB_setSw 		(PCB_p this, unsigned long sw); 
unsigned long 	PCB_getSw 		(PCB_p this, int *ptr_error);
char* 			PCB_toString 	(PCB_p this, char *str, int *ptr_error);
int 			PCB_compareTo	(PCB_p this, PCB_p other, int *ptr_error);

int 			PCB_test_main	(int argc, char** argv);

#endif

