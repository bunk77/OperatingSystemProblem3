//
// Created by chris on 5/15/2016.
//

#include "fifoq.h"
#include <stdio.h>
#include <stdlib.h>
#define get_list(q) (LINKED_LIST_iterator_get_source_list( q ->itr))
struct fifoq {
  LINKED_LIST_itrerator_p itr;
};

FIFOQp FIFOQ_construct(element_interface_t *interface) {
    FIFOQp this = malloc(sizeof(struct fifoq));
    // TODO: handle this == NULL
    LINKED_LISTp list = LINKED_LIST_construct(interface);
    // TODO: handle construct error
    this->itr = LINKED_LIST_get_itr(list);
    // TODO: handle get_itr error

    return this;
}

void  FIFOQ_enqueue(FIFOQp this, void* item) {
    // TODO: handle this == NULL
    // TODO: handle this->itr == NULL
    LINKED_LISTp list = get_list(this);
    // TODO: handle list == NULL
    LINKED_LIST_add(list, item);
}

void* FIFOQ_peek(FIFOQp this) {
    // TODO: handle this == NULL
    // TODO: handle this->itr == NULL
    return LINKED_LIST_iterator_peek(this->itr);
}

void* FIFOQ_dequeue(FIFOQp this) {
    // TODO: handle this == NULL
    // TODO: handle this->itr == NULL
    void* value = LINKED_LIST_iterator_next(this->itr);
    LINKED_LIST_iterator_remove(this->itr);
    return value;
}

void  FIFOQ_destruct(FIFOQp this) {
    // TODO: handle this == NULL
    LINKED_LISTp list = get_list(this);
    // TODO: handle list == NULL
    LINKED_LIST_destruct(list);
    LINKED_LIST_iterator_destruct(this->itr);
    free(this);
}

uint64_t FIFOQ_size(FIFOQp this) {
    // TODO: handle this == NULL
    LINKED_LISTp list = get_list(this);
    // TODO: handle list == NULL
    return LINKED_LIST_size(list);
}
