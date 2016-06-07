//
// Created by chris on 6/1/2016.
//

#include "threadq.h"
#include <stdlib.h>

/**
 * A node for a {@code THREADQp} collection.
 */
typedef struct threadq_node *threadq_node_t;

/**
 * A Linked List implementation of a FIFO queue
 */
struct threadq
{
    /**
     * The first node in the queue.
     */
    threadq_node_t first;
    /**
     * Since the first item in a {@code threadq_node_t} is a pointer to another,
     * it can be treated as a double pointer.
     */
    union
    {
        threadq_node_t last;
        threadq_node_t *follow;
    };
    /**
     * The number of elements in the queue.
     */
    uint32_t size;
};

/**
 * A node for a {@code THREADQp} collection.
 */
struct threadq_node
{
    /** the next node in the list. */
    threadq_node_t next;
    /** the value held in this node. */
    thread_type value;
};

/**
 * Constructs a new Linked List implemented FIFO queue for storing
 * {@code thread_type} objects.
 *
 * @param ptr_error a memory location to store error messages.
 * @return a new {@code THREADQp} object.
 */
THREADQp THREADQ_construct(uint64_t *ptr_error)
{
    THREADQp this = malloc(sizeof(struct threadq));
    this->first = NULL;
    this->follow = &this->first;
    this->size = 0;
    return this;
}

/**
 * Clears a {@code THREADQp} object and frees it.
 *
 * @param this the queue to be destroyed.
 * @param ptr_error a memory location to store error messages.
 */
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

/**
 * Dequeues the first element from {@code this}.
 *
 * @param this the queue from which you want the thread dequeued.
 * @param ptr_error a memory location to store error messages.
 *
 * @return the thread previously stored at the front of the queue.
 */
thread_type THREADQ_dequeue(THREADQp this, uint64_t *ptr_error)
{
    threadq_node_t second = this->first->next;
    thread_type value = this->first->value;

    free(this->first);
    this->size--;

    this->first = second;

    return value;
}


/**
 * Returns the first element from {@code this}, without removing it from the
 * queue.
 *
 * @param this the queue at which you want to peek.
 * @param ptr_error a memory location to store error messages.
 *
 * @return the thread stored at the front of the queue.
 */
thread_type THREADQ_peek(THREADQp this, uint64_t *ptr_error)
{
    return this->size == 0 ? NULL : this->first->value;
}

/**
 * Enqueues the first element from {@code this}.
 *
 * @param this the queue into which you want the thread value enqueued.
 * @param value the {@code value} to add to be added to the queue.
 * @param ptr_error a memory location to store error messages.
 *
 * @return the thread previously stored at the front of the queue.
 */
void THREADQ_enqueue(THREADQp this, thread_type value, uint64_t *ptr_error)
{
    threadq_node_t new_node = malloc(sizeof(struct threadq_node));
    *this->follow = new_node;
    new_node->value = value;
    this->size++;
}

/**
 * Return a {@code bool} value depending on whether or not the queue is empty.
 *
 * @param this the queue to test.
 * @param ptr_error a memory location to store error messages.
 *
 * @return {@code true} if {@code this} has no elements, {@code false}
 *          otherwise.
 */
bool THREADQ_is_empty(THREADQp this, uint64_t *ptr_error)
{
    return (bool) (this->size == 0);
}
