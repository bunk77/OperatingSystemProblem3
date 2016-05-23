//
// Created by chris on 5/15/2016.
//

#ifndef THREADS_PCB_H
#define THREADS_PCB_H

#include <stdbool.h>
#include <time.h>
#include <inttypes.h>

enum state_type {new, ready, running, interrupted, waiting, halted};

typedef struct PCB *PCB_p;

// returns a pcb pointer to heap allocation
PCB_p PCB_construct (void);

// deallocates pcb from the heap
void PCB_destruct (PCB_p this);

// sets default values for member data
PCB_p PCB_init (PCB_p this);
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

#endif //THREADS_PCB_H
