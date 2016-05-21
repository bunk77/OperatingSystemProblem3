//
// Created by chris on 5/15/2016.
//

#include "pcb.h"
#include <stdlib.h>

#define TRAP_COUNT 2
#define TRAP_COUNT_MAX 4
#define MAX_PRIORITY 4
struct trap_call {

};

struct PCB {
  /**
   * process ID #, a unique number
   */
  uint64_t pid;
  /**
   * process state (running waiting etc.)
   */
  enum state_type state;
  /**
   * priorities 0=highest, 4=lowest
   */
  uint64_t priority;
  /**
   * holds the current pc value when preempted
   */
  uint64_t pc;
  /**
   * **unknown**
   */
  uint64_t sw;
  /**
   * This is the integer that gives the number of instructions that should be
   * processed before resetting the PC to 0. if the number is 2345 then after
   * pc reaches 2345 it is reset to 0.
   */
  uint64_t max_pc;
  /**
   * Computer clock time when the process is created. the time when the
   * process is instantiated is captured and stored it here.
   */
  clock_t creation;
  /**
   * Computer clock time when the process is terminated (is placed into the
   * termination list). Until this process terminates this field is set to 0.
   */
  clock_t termination;
  /**
   * The total number of times that this process has run (the number of times
   * that max_pc has been reached and pc has been reset.
   */
  uint64_t term_count;
  /**
   * The max term_count value (or 0 if process is non-terminating).
   */
  uint64_t terminate;
  //??????
  struct trap_call io_traps[TRAP_COUNT][TRAP_COUNT_MAX];
};


// returns a pcb pointer to heap allocation
PCB_p PCB_construct (void) {
    PCB_p this = malloc(sizeof(struct PCB));
    // TODO: handle NULL this
    return this;
}

// deallocates pcb from the heap
void PCB_destruct (PCB_p this) {
    free(this);

}

// sets default values for member data
PCB_p PCB_init (PCB_p this) {
    // TODO: handle NULL this
    this->pc = 0;
    this->pid = (uint64_t) this;
    this->state = new;

    this->priority = 0;
    this->creation = clock();
    this->termination = 0;
    this->max_pc = (uint64_t) rand() % 1000; //TODO: verify max_pc seed
    return this;
}



#ifdef NOT_DEF
// sets pid of the process
void PCB_set_pid (PCB_p this, uint64_t pid);
// returns pid of the process
uint64_t PCB_get_pid (PCB_p this);

// sets state of the process
void PCB_set_state (PCB_p this, enum state_type state);
// returns state of the process
enum state_type PCB_get_state (PCB_p this);

// sets priority of the process
void PCB_set_priority (PCB_p this, uint64_t priority);
// returns priority of the process
uint64_t PCB_get_priority (PCB_p this);

// sets pc of the process
void PCB_set_pc (PCB_p this, uint64_t pc);
// returns pc of the process
uint64_t PCB_get_pc (PCB_p this);

// sets sw of the process
void PCB_set_sw (PCB_p this, uint64_t sw);
// returns sw of the process
uint64_t PCB_get_sw (PCB_p this);

// sets max_pc of the process
void PCB_set_max_pc (PCB_p this, uint64_t max_pc);
// returns max_pc of the process
uint64_t PCB_get_max_pc (PCB_p this);

// sets creation time of the process
void PCB_set_creation (PCB_p this, clock_t creation);
// returns creation time of the process
clock_t PCB_get_creation (PCB_p this);

// sets termination time of the process
void PCB_set_termination (PCB_p this, clock_t termination);
// returns termination time of the process
clock_t PCB_get_termination (PCB_p this);

// sets term_count field of the process
void PCB_set_term_count (PCB_p this, uint64_t term_count);
// returns term_count field of the process
uint64_t PCB_get_term_count (PCB_p this);

// sets terminate field of the process
void PCB_set_terminate (PCB_p this, uint64_t terminate);
// returns terminate field of the process
uint64_t PCB_get_terminate (PCB_p this);

#endif

// returns a string representing the contents of the pcb
char * PCB_to_string (PCB_p this, char* buffer);

