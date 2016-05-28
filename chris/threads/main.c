
//
// Created by chris on 5/15/2016.
//
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "linked_list.h"
#include "fifoq.h"
#include "priorityq.h"
#include "vector.h"
#include "fthread.h"

char* int_to_string(void* this, char* buffer);
int main(int argc, char* argv[]) {

    element_interface_t interface;
    interface.to_string = int_to_string;
    char buffer[1000];
    LINKED_LISTp list = LINKED_LIST_construct(&interface);
    LINKED_LIST_add(list, (void*)0);
    LINKED_LIST_add(list, (void*)1);
    LINKED_LIST_add(list, (void*)2);
    LINKED_LIST_add(list, (void*)3);
    printf("size: %ld\n", LINKED_LIST_size(list));
    printf("%s\n", LINKED_LIST_to_string(list, buffer));
    LINKED_LIST_set(list, 2, (void*) 5);
    printf("%s\n", LINKED_LIST_to_string(list, buffer));
    LINKED_LIST_insert(list, 2, (void*) 8);
    printf("%s\n", LINKED_LIST_to_string(list, buffer));
    printf("list[%d]: %p\n", 0, LINKED_LIST_get(list, 0));
    printf("list[%d]: %p\n", 1, LINKED_LIST_get(list, 1));
    printf("list[%d]: %p\n", 2, LINKED_LIST_get(list, 2));
    printf("list[%d]: %p\n", 3, LINKED_LIST_get(list, 3));
    LINKED_LIST_itrerator_p itr = LINKED_LIST_get_itr(list);
    printf("itr has next %s\n",
           bool_to_string(LINKED_LIST_iterator_has_next(itr)));
    LINKED_LIST_iterator_next(itr);


    printf("list[%d]: %p\n", 0, LINKED_LIST_get(list, 0));

    LINKED_LIST_iterator_remove(itr);

    printf("size: %ld\n", LINKED_LIST_size(list));

    printf("list[%d]: %p\n", 1, LINKED_LIST_iterator_next(itr));
    printf("list[%d]: %p\n", 1, LINKED_LIST_iterator_peek(itr));

    LINKED_LIST_remove(list, 0);
    printf("\n\nlist[%d]: %p\n", 0, LINKED_LIST_get(list, 0));
    printf("list[%d]: %p\n", 1, LINKED_LIST_get(list, 1));


    printf("itr has next %s\n",
           bool_to_string(LINKED_LIST_iterator_has_next(itr)));
    LINKED_LIST_iterator_destruct(itr);
    LINKED_LIST_destruct(list);
    // FIFOQ
    printf("\n\nFIFOQ\n");
    FIFOQp queue = FIFOQ_construct(NULL);
    printf("empty size: %ld\n", FIFOQ_size(queue));
    FIFOQ_enqueue(queue, (void*) 0);
    printf("enqueued 1 size: %ld\n", FIFOQ_size(queue));
    FIFOQ_enqueue(queue, (void*) 1);
    printf("enqueued 2 size: %ld\n", FIFOQ_size(queue));
    FIFOQ_enqueue(queue, (void*) 2);

    printf("peeked   0: %p, size: %ld\n", FIFOQ_peek(queue),
           FIFOQ_size(queue));
    printf("dequeued 0: %p, size: %ld\n", FIFOQ_dequeue(queue),
           FIFOQ_size(queue));

    printf("peeked   1: %p, size: %ld\n", FIFOQ_peek(queue),
           FIFOQ_size(queue));
    printf("dequeued 1: %p, size: %ld\n", FIFOQ_dequeue(queue),
           FIFOQ_size(queue));

    printf("peeked   2: %p, size: %ld\n", FIFOQ_peek(queue),
           FIFOQ_size(queue));
    printf("dequeued 2: %p, size: %ld\n", FIFOQ_dequeue(queue),
           FIFOQ_size(queue));
    printf("size: %ld\n", FIFOQ_size(queue));
    FIFOQ_destruct(queue);
    // PRIORITY QUEUE
    printf("PRIORITY QUEUE\n");
    PRIORITYQp pqueue = PRIORITYQ_construct(NULL);
    printf("empty size: %ld\n", PRIORITYQ_size(pqueue));
    PRIORITYQ_enqueue(pqueue, (void*) 5, 3);
    printf("enqueued (5, 3) size: %ld\n", PRIORITYQ_size(pqueue));
    PRIORITYQ_enqueue(pqueue, (void*) 6, 1);
    printf("enqueued (6, 1) size: %ld\n", PRIORITYQ_size(pqueue));
    PRIORITYQ_enqueue(pqueue, (void*) 7, 2);
    printf("enqueued (7, 2) size: %ld\n", PRIORITYQ_size(pqueue));
    int i;
    for (i = 0; i < 2; i++) {
        printf("               peeked %d: %p,     size: %ld\n",
               i, PRIORITYQ_peek(pqueue), PRIORITYQ_size(pqueue));
        printf("new size %2ld, dequeued %d: %p, old size: %ld\n",
               PRIORITYQ_size(pqueue), i, PRIORITYQ_dequeue(pqueue),
               PRIORITYQ_size(pqueue));
    }

    PRIORITYQ_destruct(pqueue);
    printf("\n\n\n\n\n\nVECTOR_LIST\n");
    // VECTOR
    VECTOR_LISTp vlist = VECTOR_LIST_construct(&interface);
    VECTOR_LIST_add(vlist, (void*)0);
    printf("%s\n", VECTOR_LIST_to_string(vlist, buffer));
    VECTOR_LIST_add(vlist, (void*)1);
    printf("%s\n", VECTOR_LIST_to_string(vlist, buffer));
    VECTOR_LIST_add(vlist, (void*)2);
    printf("%s\n", VECTOR_LIST_to_string(vlist, buffer));
    VECTOR_LIST_add(vlist, (void*)3);
    printf("size: %ld\n", VECTOR_LIST_size(vlist));
    printf("%s\n", VECTOR_LIST_to_string(vlist, buffer));
    VECTOR_LIST_set(vlist, 2, (void*) 5);
    printf("%s\n", VECTOR_LIST_to_string(vlist, buffer));
    VECTOR_LIST_insert(vlist, 2, (void*) 8);
    printf("%s\n", VECTOR_LIST_to_string(vlist, buffer));
    printf("list[%d]: %p\n", 0, VECTOR_LIST_get(vlist, 0));
    printf("list[%d]: %p\n", 1, VECTOR_LIST_get(vlist, 1));
    printf("list[%d]: %p\n", 2, VECTOR_LIST_get(vlist, 2));
    printf("list[%d]: %p\n", 3, VECTOR_LIST_get(vlist, 3));
    VECTOR_LIST_itrerator_p vitr = VECTOR_LIST_get_itr(vlist);
    printf("itr has next %s\n",
           bool_to_string(VECTOR_LIST_iterator_has_next(vitr)));
    VECTOR_LIST_iterator_next(vitr);
    //VECTOR_LIST_iterator_next(vitr);


    printf("list[%d]: %p\n", 0, VECTOR_LIST_get(vlist, 0));

    VECTOR_LIST_iterator_remove(vitr);

    printf("removed:%s\n", VECTOR_LIST_to_string(vlist, buffer));
    printf("size: %ld\n", VECTOR_LIST_size(vlist));

    printf("list[%d]: %p\n", 1, VECTOR_LIST_iterator_next(vitr));
    printf("list[%d]: %p\n", 1, VECTOR_LIST_iterator_peek(vitr));

    printf("removed:%s\n", VECTOR_LIST_to_string(vlist, buffer));
    VECTOR_LIST_remove(vlist, 1);
    printf("removed:%s\n", VECTOR_LIST_to_string(vlist, buffer));
    printf("removed:%s\n", VECTOR_LIST_to_string(vlist, buffer));
    printf("\n\nlist[%d]: %p\n", 0, VECTOR_LIST_get(vlist, 0));
    printf("list[%d]: %p\n", 1, VECTOR_LIST_get(vlist, 1));


    printf("itr has next %s\n",
           bool_to_string(VECTOR_LIST_iterator_has_next(vitr)));
    VECTOR_LIST_iterator_destruct(vitr);
    VECTOR_LIST_destruct(vlist);
    clock_t t = clock();
    //fwait(NULL, 5);

    printf ("just waited %ld seconds.\n",(clock()-t)/CLOCKS_PER_SEC);
    return EXIT_SUCCESS;
}

char* int_to_string(void* this, char* buffer) {
    sprintf(buffer, "%ld", (uint64_t) this);
    return buffer;
}