/*
 * Problem 3 - Discontinuities
 * TCSS 422 A Spring 2016
 * Bun Kak, Chris Ottersen, Mark Peters, Paul Zander
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <limits.h>
#include "PCB.h"

//struct and error defines originally here
char *STATE[] = {"created", "ready", "running", "interrupted", "waiting", "terminated"};

/**
 * Returns a pcb pointer to heap allocation.
 * 
 * @return a pointer to a new PCB object
 */
PCB_p PCB_construct(int *ptr_error) {
    PCB_p this = (PCB_p) malloc(sizeof (struct PCB));
    this->regs = (REG_p) malloc(sizeof (struct regfile));
    int error = ((!this) * PCB_INIT_ERROR);
    this->pid = 0;
    this->priority = 0;
    this->state = 0;

    if (ptr_error != NULL) {
        *ptr_error += error;
    }
    return (this);
}

REG_p REG_init(REG_p this, int *ptr_error) {
    if (this == NULL)
        *ptr_error += PCB_NULL_ERROR;
    else {
        int t;
        this->pc = DEFAULT_PC;
        this->MAX_PC = rand() % (MAX_PC_RANGE + 1) + MAX_PC_MIN;
        this->sw = DEFAULT_SW;
        this->term_count = 0;
        this->TERMINATE = rand() % TERM_RANGE;
        for (t = 0; t < IO_NUMBER * IO_CALLS; t++)
            this->IO_TRAPS[(int) (t / IO_CALLS)][t % IO_CALLS] = rand() % this->MAX_PC;
    }
}

PCB_p PCB_construct_init(int *ptr_error) {
    PCB_p pcb = PCB_construct(ptr_error);
    *ptr_error += PCB_init(pcb);
    return pcb;
}

/**
 * Deallocates pcb from the heap.
 * @param this
 */
int PCB_destruct(PCB_p this) {
    int error = (this == NULL) * PCB_NULL_ERROR; // sets error code to 1 if `this` is NULL
    if (!error) {
        if (this->regs != NULL)
            free(this->regs);
        free(this);
        this = NULL;
    }
    return error;
}

/**
 * Sets default values for member data.
 * @param this
 * @return 
 */
int PCB_init(PCB_p this) {
    static unsigned long pidCounter = ULONG_MAX;
    static int firstCall = 1;
    if (!firstCall) {
        srand(time(NULL) << 1);
        firstCall = 0;
    }
    int error = (this == NULL) * PCB_NULL_ERROR;
    int t;
    if (!error) {
        REG_init(this->regs, error);
        this->pid = ++pidCounter;
        this->priority = rand() & LOWEST_PRIORITY;
        this->state = DEFAULT_STATE;
        this->timeCreate = clock();
        this->timeTerminate = 0;
    }
    return error;
}

/**
 * Sets the pid of the process.
 * 
 * @param this 
 * @param pid the new pid value
 * @return 
 */
int PCB_setPid(PCB_p this, unsigned long pid) {
    // verify pid does not already exist
    int error = (this == NULL) * PCB_NULL_ERROR;
    if (!error) {
        this->pid = pid;
    }
    return error; // TODO: write
}

/**
 * Returns the pid of the process.
 * 
 * @param this
 * @return the pid of the process
 */
unsigned long PCB_getPid(PCB_p this, int *ptr_error) {
    int error = (this == NULL) * PCB_NULL_ERROR;

    if (ptr_error != NULL) {
        *ptr_error += error;
    }

    return error ? ~0 : this->pid; // TODO: write
}

/**
 * Sets the state of the process.
 * 
 * @param this 
 * @param state the new state value
 * @return error 
 */
int PCB_setState(PCB_p this, enum state_type state) {
    int error = 0;
    if (this == NULL) {
        error |= PCB_NULL_ERROR;
    }
    if (state < created || state > terminated) {
        error |= PCB_OTHER_ERROR;
    }
    if (!error) {
        this->state = state;
    }
    return error;
}

/**
 * Returns the current state of the process.
 * 
 * @param this
 * @return the state of the process
 */
enum state_type PCB_getState(PCB_p this, int *ptr_error) {
    int error = (this == NULL) * PCB_NULL_ERROR;

    if (ptr_error != NULL) {
        *ptr_error += error;
    }
    return error ? ~0 : this->state;
}

/**
 * Sets the pid of the process.
 * 
 * @param this 
 * @param pid the new pid value
 * @return 
 */
int PCB_setPriority(PCB_p this, unsigned short priority) {
    int error = 0;
    if (this == NULL) {
        error |= PCB_NULL_ERROR;
    }
    if (priority > LOWEST_PRIORITY) {
        error |= PCB_OTHER_ERROR;
    }
    if (!error) {
        this->priority = priority;
    }
    return error;
}

/**
 * Returns the current state of the process.
 * 
 * @param this
 * @return the pid of the process
 */
unsigned short PCB_getPriority(PCB_p this, int *ptr_error) {
    int error = (this == NULL) * PCB_NULL_ERROR;

    if (ptr_error != NULL) {
        *ptr_error += error;
    }
    return error ? ~0 : this->priority; // TODO: write
}

/**
 * Sets the pid of the process.
 * 
 * @param this 
 * @param pid the new pid value
 * @return 
 */
int PCB_setPc(PCB_p this, unsigned long pc) {
    int error = (this == NULL) * PCB_NULL_ERROR;
    if (!error) {
        this->regs->pc = pc;
    }
    return error; // TODO: write
}

/**
 * Returns the pc value state of the process.
 * 
 * @param this
 * @return the address where the program will resume
 */
unsigned long PCB_getPc(PCB_p this, int *ptr_error) {
    int error = (this == NULL) * PCB_NULL_ERROR;

    if (ptr_error != NULL) {
        *ptr_error += error;
    }
    return error ? -1 : this->regs->pc; // TODO: write
}

/**
 * Sets the pid of the process.
 * 
 * @param this 
 * @param pid the new pid value
 * @return 
 */
int PCB_setSw(PCB_p this, unsigned long sw) {
    int error = (this == NULL) * PCB_NULL_ERROR;
    if (!error) {
        this->regs->sw = sw;
    }
    return error; // TODO: write
}

/**
 * Returns the pc value state of the process.
 * 
 * @param this
 * @return the address where the program will resume
 */
unsigned long PCB_getSw(PCB_p this, int *ptr_error) {
    int error = (this == NULL) * PCB_NULL_ERROR;
    if (ptr_error != NULL) {
        *ptr_error += error;
    }
    return error ? ~0 : this->regs->sw; // TODO: write
}

/**
 * Returns a string representing the contents of the pcb. 
 * <br /><em><strong>Note:</strong> parameter string must be 80 chars or more.</em>
 * @param this
 * @param str
 * @param int
 * @return string representing the contents of the pc.
 */
char * PCB_toString(PCB_p this, char *str, int *ptr_error) {
    int error = (this == NULL || str == NULL) * PCB_NULL_ERROR;
    if (!error) {
        str[0] = '\0';
        const char * format = "PID: 0x%04lx  PC: 0x%05lx  State: %s  Priority 0x%x";
        snprintf(str, (size_t) PCB_TOSTRING_LEN - 1, format, this->pid, this->regs->pc, STATE[this->state], this->priority);
    }

    if (ptr_error != NULL) {
        *ptr_error += error;
    }
    return str;

}