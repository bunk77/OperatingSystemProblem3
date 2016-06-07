//
// Created by chris on 6/1/2016.
//

#ifndef THREADS_THREADQ_H
#define THREADS_THREADQ_H

#include <stdbool.h>
#include <inttypes.h>

typedef struct PCB *thread_type;

/**
 * A Linked List implementation of a FIFO queue
 */
typedef struct threadq *THREADQp;

/**
 * Constructs a new Linked List implemented FIFO queue for storing
 * {@code thread_type} objects.
 *
 * @param ptr_error a memory location to store error messages.
 * @return a new {@code THREADQp} object.
 */
THREADQp THREADQ_construct(uint64_t *ptr_error);

/**
 * Clears a {@code THREADQp} object and frees it.
 *
 * @param this the queue to be destroyed.
 * @param ptr_error a memory location to store error messages.
 */
void THREADQ_destruct(THREADQp this, uint64_t *ptr_error);

/**
 * Dequeues the first element from {@code this}.
 *
 * @param this the queue from which you want the thread dequeued.
 * @param ptr_error a memory location to store error messages.
 *
 * @return the thread previously stored at the front of the queue.
 */
thread_type THREADQ_dequeue(THREADQp this, uint64_t *ptr_error);

/**
 * Returns the first element from {@code this}, without removing it from the
 * queue.
 *
 * @param this the queue at which you want to peek.
 * @param ptr_error a memory location to store error messages.
 *
 * @return the thread stored at the front of the queue.
 */
thread_type THREADQ_peek(THREADQp this, uint64_t *ptr_error);

/**
 * Enqueues the first element from {@code this}.
 *
 * @param this the queue into which you want the thread value enqueued.
 * @param value the {@code value} to add to be added to the queue.
 * @param ptr_error a memory location to store error messages.
 *
 * @return the thread previously stored at the front of the queue.
 */
void THREADQ_enqueue(THREADQp this, thread_type value, uint64_t *ptr_error);

/**
 * Return a {@code bool} value depending on whether or not the queue is empty.
 *
 * @param this the queue to test.
 * @param ptr_error a memory location to store error messages.
 *
 * @return {@code true} if {@code this} has no elements, {@code false}
 *          otherwise.
 */
bool THREADQ_is_empty(THREADQp this, uint64_t *ptr_error);


#endif //THREADS_THREADQ_H
