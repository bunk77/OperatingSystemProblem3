//
// Created by chris on 6/1/2016.
//

#include "threadq.h"

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
    PCB_p value;
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

PCB_p THREADQ_dequeue(THREADQp this, bool requeue, uint64_t *ptr_error)
{
    threadq_node_t second = this->first->next;
    PCB_p value = this->first->value;
    if (requeue) {
        *this->follow = this->first;
    } else {
        free(this->first);
        this->size--;
    }
    this->first = second;

    return value;
}

PCB_p THREADQ_peek(THREADQp this, uint64_t *ptr_error)
{
    return this->first->value;
}

void THREADQ_enqueue(THREADQp this, PCB_p value, uint64_t *ptr_error)
{
    threadq_node_t new_node = malloc(sizeof(struct threadq_node));

    *this->follow = new_node;
    this->size++;
}

bool THREADQ_is_empty(THREADQp this, uint64_t *ptr_error)
{
    return this->size == 0;
}