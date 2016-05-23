//
// Created by chris on 5/15/2016.
//

#ifndef THREADS_LINKED_LIST_H
#define THREADS_LINKED_LIST_H

#include "list.h"

//typedef struct element_interface element_interface_t;
typedef struct linked_list* LINKED_LISTp;
typedef struct linked_list_iterator* LINKED_LIST_itrerator_p;

LINKED_LISTp LINKED_LIST_construct(element_interface_t *interface);
void  LINKED_LIST_add(LINKED_LISTp this, void* value);

void* LINKED_LIST_remove(LINKED_LISTp this, uint64_t i);
void LINKED_LIST_set(LINKED_LISTp this, uint64_t i, void* value);
void LINKED_LIST_insert(LINKED_LISTp this, uint64_t i, void* value);

char* LINKED_LIST_to_string(LINKED_LISTp this, char* buffer);

void* LINKED_LIST_get(LINKED_LISTp this, uint64_t i);
void  LINKED_LIST_destruct(LINKED_LISTp this);
uint64_t LINKED_LIST_size(LINKED_LISTp this);

LINKED_LIST_itrerator_p LINKED_LIST_get_itr(LINKED_LISTp this);
bool  LINKED_LIST_iterator_has_next(LINKED_LIST_itrerator_p this);
void* LINKED_LIST_iterator_next(LINKED_LIST_itrerator_p this);
void* LINKED_LIST_iterator_peek(LINKED_LIST_itrerator_p this);
void  LINKED_LIST_iterator_remove(LINKED_LIST_itrerator_p this);
LINKED_LISTp LINKED_LIST_iterator_get_source_list(LINKED_LIST_itrerator_p this);
void  LINKED_LIST_iterator_destruct(LINKED_LIST_itrerator_p this);

#endif //THREADS_LINKED_LIST_H
