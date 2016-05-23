//
// Created by chris on 5/15/2016.
//
#ifndef THREADS_FIFOQ_H
#define THREADS_FIFOQ_H

#include "linked_list.h"
#include "list.h"

typedef struct fifoq *FIFOQp;

FIFOQp FIFOQ_construct(element_interface_t *interface);
void  FIFOQ_enqueue(FIFOQp this, void* item);
void* FIFOQ_dequeue(FIFOQp this);
void* FIFOQ_peek(FIFOQp this);

void  FIFOQ_destruct(FIFOQp this);
uint64_t FIFOQ_size(FIFOQp this);


#endif //THREADS_FIFOQ_H