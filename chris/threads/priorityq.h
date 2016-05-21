//
// Created by chris on 5/18/2016.
//

#ifndef THREADS_PRIORITYQ_H
#define THREADS_PRIORITYQ_H
#include "fifoq.h"
struct priority_interface {
  uint64_t priority;
};
typedef struct priority_interface priority_interface_t;
typedef struct priorityq *PRIORITYQp;

PRIORITYQp PRIORITYQ_construct(element_interface_t *interface);
void  PRIORITYQ_enqueue(PRIORITYQp this, void* item, uint64_t priority);

void* PRIORITYQ_peek(PRIORITYQp this);
void* PRIORITYQ_dequeue(PRIORITYQp this);
uint64_t PRIORITYQ_size(PRIORITYQp this);

void  PRIORITYQ_destruct(PRIORITYQp this);

#endif //THREADS_PRIORITYQ_H
