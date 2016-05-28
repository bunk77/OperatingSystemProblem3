//
// Created by chris on 5/18/2016.
//

#include "priorityq.h"
#include <stdlib.h>
#include <time.h>
#define DEFAULT_VECTOR_SIZE 4


struct priorityq {
  FIFOQp *vector;
  uint64_t vector_size;
  uint64_t size;
  element_interface_t *interface;
};

struct priorityq_node {
  void* item;
  uint64_t priority;
  time_t accessed;
};
typedef struct priorityq_node * priorityq_node_t;

PRIORITYQp PRIORITYQ_construct(element_interface_t *interface) {
    uint64_t i;
    PRIORITYQp this = malloc(sizeof(struct priorityq));
    this->interface = interface;
    this->vector_size = DEFAULT_VECTOR_SIZE;
    this->vector = malloc(sizeof(FIFOQp) * this->vector_size);
    this->size = 0;
    for(i = 0; i < this->vector_size; i++) {
        this->vector[i] = FIFOQ_construct(interface);
    }
    return this;
}

void  PRIORITYQ_enqueue(PRIORITYQp this, void* item, uint64_t priority) {
    //lock priority queue
    // HINT: makes queue resizeable
    if (this->vector_size < priority) {
        uint64_t i;
        uint64_t new_size = priority << 2;
        this->vector = realloc(this->vector, sizeof(FIFOQp) * new_size);
        for(i = this->vector_size; i < new_size; i++) {
            this->vector[i] = FIFOQ_construct(this->interface);
        }
    }
    priorityq_node_t node = malloc(sizeof(struct priorityq_node));
    node->item = item;
    node->priority = priority;
    node->accessed = time(NULL);
    this->size++;
    FIFOQ_enqueue(this->vector[node->priority], node);
}

void* PRIORITYQ_peek(PRIORITYQp this) {
    if(this->size > 0) {
        uint64_t i;
        for(i = 0; i < this->vector_size; i++) {
            if (FIFOQ_size(this->vector[i]) != 0) {
                return ((priorityq_node_t) FIFOQ_peek(this->vector[i]))->item;
            }

        }
    }
    // TODO:handle no elements found
    return NULL;
}

void* PRIORITYQ_dequeue(PRIORITYQp this) {
    // TODO: handle this == NULL
    if(this->size > 0) {
        uint64_t i;
        for(i = 0; i < this->vector_size; i++) {
            if (FIFOQ_size(this->vector[i]) != 0) {
                priorityq_node_t node = FIFOQ_dequeue(this->vector[i]);
                void* item = node->item;
                free(node);
                this->size--;
                return item;
            }
        }
    }
    // TODO:handle no elements found
    return NULL;
}

void  PRIORITYQ_destruct(PRIORITYQp this) {
    // TODO: handle this == NULL
    uint64_t i;
    for(i = 0; i < this->vector_size; i++) {
        FIFOQ_destruct(this->vector[i]);
    }
    free(this->vector);
    free(this);
}

uint64_t PRIORITYQ_size(PRIORITYQp this) {
    // TODO: handle this == NULL
    return this->size;
}

