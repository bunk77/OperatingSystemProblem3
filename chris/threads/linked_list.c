//
// Created by chris on 5/15/2016.
//

#include "linked_list.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct linked_list_node* linked_list_node_p;

struct linked_list {
  linked_list_node_p first;
  union {
    linked_list_node_p last;
    linked_list_node_p *follow;
  };
  uint32_t size;
  element_interface_t *interface;

};
struct linked_list_node {
  linked_list_node_p next;
  void* value;
};

struct linked_list_iterator {
  linked_list_node_p *next;
  LINKED_LISTp list;
  bool primed;
  uint64_t index;
};

LINKED_LISTp LINKED_LIST_construct(element_interface_t *interface) {
    LINKED_LISTp this = malloc(sizeof(struct linked_list));
    //linked_list_node_p dummy = malloc(sizeof(struct linked_list_node));
    this->first = NULL;
    this->follow = &this->first;
    this->size = 0;
    this->interface = interface;
    return this;
}

void LINKED_LIST_add(LINKED_LISTp this, void* value) {
    linked_list_node_p new_node = malloc(sizeof(struct linked_list_node));
    new_node->value = value;
    new_node->next = NULL;
    *this->follow = new_node;
    this->follow = &new_node->next;
    this->size++;
}


void LINKED_LIST_set(LINKED_LISTp this, uint64_t i, void* value) {
    linked_list_node_p itr_node;
    if (i == this->size - 1) {
        itr_node = this->last;
    } else {
        itr_node = this->first->next;
        while(i-- > 0) {
            itr_node = itr_node->next;
        }
    }
    itr_node->value = value;
}

void LINKED_LIST_insert(LINKED_LISTp this, uint64_t i, void* value) {

    linked_list_node_p new_node = malloc(sizeof(struct linked_list_node));
    new_node->value = value;
    
    linked_list_node_p *itr_node;
    if (i == this->size - 1) {
        itr_node = this->follow;
    } else {
        itr_node = &this->first;
        while (i-- > 0) {
            itr_node = &(*itr_node)->next;
        }
    }
    new_node->next = *itr_node;
    *itr_node = new_node;
    this->size++;
}

void* LINKED_LIST_remove(LINKED_LISTp this, uint64_t i) {
    void* value = NULL;
    linked_list_node_p *itr_node = &this->first;
    linked_list_node_p node_found;
    while(i-- > 0) {
        itr_node = &(*itr_node)->next;
    }
    node_found = *itr_node;
    *itr_node = node_found->next;
    value = node_found->value;
    free(node_found);

    this->size--;
    return value;
}

void* LINKED_LIST_get(LINKED_LISTp this, uint64_t i) {
    linked_list_node_p itr_node;
    if (i == this->size - 1) {
        itr_node = this->last;
    } else {
        itr_node = this->first;
        while(i-- > 0) {
            itr_node = itr_node->next;
        }
    }
    return itr_node->value;
}

void LINKED_LIST_destruct(LINKED_LISTp this) {
    while(this->first != NULL) {
        linked_list_node_p tmp = this->first;
        this->first = this->first->next;
        this->size--;
        free(tmp);
    }

    free(this);
}

uint64_t LINKED_LIST_size(LINKED_LISTp this) {
    return this->size;
}

// BEGINNING OF ITERATOR METHODS
LINKED_LIST_itrerator_p LINKED_LIST_get_itr(LINKED_LISTp this) {
    LINKED_LIST_itrerator_p new_itr = malloc(sizeof(struct linked_list_iterator));
    new_itr->next=&this->first;
    new_itr->list=this;
    new_itr->primed=false;
    new_itr->index=0;
    return new_itr;
}

void* LINKED_LIST_iterator_next(LINKED_LIST_itrerator_p this) {
    if(this->primed) {
        this->next = &(*this->next)->next;
    } else {
        this->primed = true;
    }
    this->index++;

    void* value = (*this->next)->value;
    return value;
}

void* LINKED_LIST_iterator_peek(LINKED_LIST_itrerator_p this) {
    return (*this->next)->value;
}

void LINKED_LIST_iterator_remove(LINKED_LIST_itrerator_p this) {
    if(this->primed) {
        linked_list_node_p next = (*this->next)->next;
        free(*this->next);
        this->primed = false;
        *this->next = next;
        this->list->size--;
    } else {
        fprintf(stderr, "ILLEGAL_STATE_EXCEPTION: the next method has not yet"
            " been called, or the remove method has already been called after"
            " the last call to the next method\n");
    }
}

LINKED_LISTp LINKED_LIST_iterator_get_source_list(LINKED_LIST_itrerator_p this) {
    return this->list;
}

bool LINKED_LIST_iterator_has_next(LINKED_LIST_itrerator_p this) {
    return (*this->next)->next != NULL;
}

void LINKED_LIST_iterator_destruct(LINKED_LIST_itrerator_p this) {
    free(this);
}

char* LINKED_LIST_to_string(LINKED_LISTp this, char* buffer) {
    linked_list_node_p itr_node = this->first;
    char tmp[1000];
    uint64_t len;
    buffer[0] = '[';
    buffer[1] = '\0';
    while(itr_node != NULL) {
        strcat(buffer, this->interface->to_string(itr_node->value, tmp));
        strcat(buffer, ", ");
        itr_node = itr_node->next;
    }
    len = strlen(buffer);
    buffer[len - 2] = ']';
    buffer[len - 1] = '\0';
    return buffer;

}
