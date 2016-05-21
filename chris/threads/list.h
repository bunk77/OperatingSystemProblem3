//
// Created by chris on 5/18/2016.
//

#ifndef THREADS_LIST_H
#define THREADS_LIST_H
#include <stdint.h>
#include <stdbool.h>
#define bool_to_string(arg) (arg ? "true" : "false")
struct element_interface {
  char* (*to_string) (void* this, char* buffer);
  void (*destruct) (void* this);
};
typedef struct element_interface element_interface_t;
#endif //THREADS_LIST_H
