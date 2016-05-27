//
// Created by chris on 5/17/2016.
//

#ifndef THREADS_VECTOR_H
#define THREADS_VECTOR_H

#include <stdint.h>
#include <stdbool.h>

typedef struct element_interface element_interface_t;
typedef struct vector_list* VECTOR_LISTp;
typedef struct vector_list_iterator* VECTOR_LIST_itrerator_p;

VECTOR_LISTp VECTOR_LIST_construct(element_interface_t *interface);
void  VECTOR_LIST_set(VECTOR_LISTp this, uint64_t i, void* value);
void  VECTOR_LIST_insert(VECTOR_LISTp this, uint64_t i, void* value);

void  VECTOR_LIST_add(VECTOR_LISTp this, void* value);
void* VECTOR_LIST_remove(VECTOR_LISTp this, uint64_t i);
void* VECTOR_LIST_get(VECTOR_LISTp this, uint64_t i);
void  VECTOR_LIST_destruct(VECTOR_LISTp this);
uint64_t VECTOR_LIST_size(VECTOR_LISTp this);

char* VECTOR_LIST_to_string(VECTOR_LISTp this, char* buffer);


VECTOR_LIST_itrerator_p VECTOR_LIST_get_itr(VECTOR_LISTp this);
bool  VECTOR_LIST_iterator_has_next(VECTOR_LIST_itrerator_p this);
void* VECTOR_LIST_iterator_next(VECTOR_LIST_itrerator_p this);
void* VECTOR_LIST_iterator_peek(VECTOR_LIST_itrerator_p this);
void  VECTOR_LIST_iterator_remove(VECTOR_LIST_itrerator_p this);
void  VECTOR_LIST_iterator_destruct(VECTOR_LIST_itrerator_p this);


#endif //THREADS_VECTOR_H
