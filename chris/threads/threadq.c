//
// Created by chris on 6/1/2016.
//

#include "threadq.h"
#include <stdlib.h>
typedef struct threadq_node *threadq_node_t;
struct threadq
{
    threadq_node_t first;
    union
    {
        threadq_node_t last;
        threadq_node_t *follow;
    };
    uint32_t size;

};

struct threadq_node
{
    threadq_node_t next;
    thread_type value;
};

THREADQp THREADQ_construct(uint64_t *ptr_error)
{
    THREADQp this = malloc(sizeof(struct threadq));
    //linked_list_node_p dummy = malloc(sizeof(struct linked_list_node));
    this->first = NULL;
    this->follow = &this->first;
    this->size = 0;
    return this;
}

void THREADQ_destruct(THREADQp this, uint64_t *ptr_error)
{
    while (this->first != NULL) {
        threadq_node_t tmp = this->first;
        this->first = this->first->next;
        this->size--;
        free(tmp);
    }

    free(this);
}

thread_type THREADQ_dequeue(THREADQp this, bool requeue, uint64_t *ptr_error)
{
    threadq_node_t second = this->first->next;
    thread_type value = this->first->value;
    if (requeue) {
        *this->follow = this->first;
    } else {
        free(this->first);
        this->size--;
    }
    this->first = second;

    return value;
}

thread_type THREADQ_peek(THREADQp this, uint64_t *ptr_error)
{
    return this->size == 0 ? NULL : this->first->value;
}

void THREADQ_enqueue(THREADQp this, thread_type value, uint64_t *ptr_error)
{
    threadq_node_t new_node = malloc(sizeof(struct threadq_node));

    *this->follow = new_node;
    this->size++;
}

bool THREADQ_is_empty(THREADQp this, uint64_t *ptr_error)
{
    return (bool) (this->size == 0);
}