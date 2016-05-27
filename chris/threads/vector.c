//
// Created by chris on 5/17/2016.
//
#include "vector.h"
#include "list.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//typedef struct linked_list_node* linked_list_node_p;

#define DEFAULT_VECTOR_SIZE 4

struct vector_list {
  void **vector;
  uint64_t vector_size;
  uint64_t size;
  element_interface_t *interface;
};

struct vector_list_iterator {
  void *next;
  VECTOR_LISTp list;
  bool primed;
  uint64_t index;
};

static void vector_list_expand(VECTOR_LISTp this);

VECTOR_LISTp VECTOR_LIST_construct(element_interface_t *interface) {
    VECTOR_LISTp this = malloc(sizeof(struct vector_list));
    this->interface = interface;
    this->vector_size = DEFAULT_VECTOR_SIZE;
    this->vector = malloc(sizeof(void*) * this->vector_size);
    this->size = 0;
    return this;
}

void  VECTOR_LIST_add(VECTOR_LISTp this, void* value) {
    this->size++;
    vector_list_expand(this);
    this->vector[this->size-1] = value;
}


void  VECTOR_LIST_set(VECTOR_LISTp this, uint64_t i, void* value) {
    this->vector[i] = value;
}

void  VECTOR_LIST_insert(VECTOR_LISTp this, uint64_t i, void* value) {
    uint64_t j = ++this->size;
    vector_list_expand(this);

    memmove(this->vector + i + 1,
            this->vector + i,
            sizeof(void*) * this->size);
    this->vector[i] = value;
}

void* VECTOR_LIST_remove(VECTOR_LISTp this, uint64_t i) {
    void* value = this->vector[--i];
    this->size--;
    memmove(this->vector + i, this->vector + i + 1, sizeof(void*) * (this->size - i));
    return value;
}

void* VECTOR_LIST_get(VECTOR_LISTp this, uint64_t i) {
    return this->vector[i];
}


void VECTOR_LIST_destruct(VECTOR_LISTp this) {
    free(this->vector);
    free(this);
}
uint64_t VECTOR_LIST_size(VECTOR_LISTp this) {
    return this->size;
}


char* VECTOR_LIST_to_string(VECTOR_LISTp this, char* buffer) {
    char tmp[1000];
    uint64_t len;
    uint64_t i;

    buffer[0] = '[';
    buffer[1] = '\0';
    for(i = 0; i < this->size; i++) {

        strcat(buffer, this->interface->to_string(this->vector[i],tmp));
        strcat(buffer, ", ");
    }
    len = strlen(buffer);
    buffer[len - 2] = ']';
    buffer[len - 1] = '\0';
    return buffer;

}


// BEGINNING OF ITERATOR METHODS
VECTOR_LIST_itrerator_p VECTOR_LIST_get_itr(VECTOR_LISTp this) {
    VECTOR_LIST_itrerator_p new_itr = malloc(sizeof(struct vector_list_iterator));
    new_itr->next = &this->vector;
    new_itr->list = this;
    new_itr->primed = false;
    new_itr->index = 0;
    return new_itr;
}

void* VECTOR_LIST_iterator_next(VECTOR_LIST_itrerator_p this) {
    if(this->primed) {
        this->index++;
    } else {
        this->primed = true;
    }
    return this->list->vector[this->index];
}

void* VECTOR_LIST_iterator_peek(VECTOR_LIST_itrerator_p this) {
    return this->list->vector[this->index + !this->primed];
}

void VECTOR_LIST_iterator_remove(VECTOR_LIST_itrerator_p this) {
    if(this->primed) {
        VECTOR_LIST_remove(this->list, this->index);
        this->primed = false;
        this->list->size--;

    } else {
        fprintf(stderr, "ILLEGAL_STATE_EXCEPTION: the next method has not yet"
            " been called, or the remove method has already been called after"
            " the last call to the next method\n");
        //exit(0);
    }
}

VECTOR_LISTp VECTOR_LIST_iterator_get_source_list(VECTOR_LIST_itrerator_p this) {
    return this->list;
}

bool VECTOR_LIST_iterator_has_next(VECTOR_LIST_itrerator_p this) {
    return this->index < this->list->size;
}

void VECTOR_LIST_iterator_destruct(VECTOR_LIST_itrerator_p this) {
    free(this);
}

static void vector_list_expand(VECTOR_LISTp this) {
    if (this->vector_size < this->size) {
        this->vector_size <<= 2;
        void** tmp = malloc(sizeof(void*) * this->vector_size);

        memcpy(tmp, this->vector, sizeof(void*) * this->size);
        free(this->vector);
        this->vector = tmp;
    }
}
