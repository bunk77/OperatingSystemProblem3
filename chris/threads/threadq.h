//
// Created by chris on 6/1/2016.
//

#ifndef THREADS_THREADQ_H
#define THREADS_THREADQ_H

#include <stdbool.h>
#include <inttypes.h>
#include "pcb.h"

typedef struct threadq *THREADQp;

THREADQp THREADQ_construct(uint64_t *ptr_error);

void THREADQ_destruct(THREADQp this, uint64_t *ptr_error);

PCB_p THREADQ_dequeue(THREADQp this, bool requeue, uint64_t *ptr_error);

void THREADQ_enqueue(THREADQp this, PCB_p value, uint64_t *ptr_error);

PCB_p THREADQ_peek(THREADQp this, uint64_t *ptr_error);

bool THREADQ_is_empty(THREADQp this, uint64_t *ptr_error);


#endif //THREADS_THREADQ_H
