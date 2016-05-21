//
// Created by chris on 5/15/2016.
//

#ifndef THREADS_ERRORS_H
#define THREADS_ERRORS_H

#include <time.h>
typedef enum error_class error_class_t;
typedef struct error *error_p;
typedef struct error_list *error_list_p;

enum error_class {

};

struct error {
  error_class_t c;
  char message[512];
  char file[256];
  unsigned int line;
  time_t time_caught;
  error_p next;
};

struct error_list {
  error_p most_recent;
};
void add_error(error_list_p this, struct error newError){
    error_p err = malloc(sizeof(struct error));
    *err = newError;
    err->next = this->most_recent;
    this->most_recent = err;
}



#endif //THREADS_ERRORS_H
