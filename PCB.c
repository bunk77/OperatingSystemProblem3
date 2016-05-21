/*
 * Problem 3 - Discontinuities
 * TCSS 422 A Spring 2016
 * Bun Kak, Chris Ottersen, Mark Peters, Paul Zander
 */

#include "PCB.h"

//struct and error defines originally here
char *STATE[] = {"created", "ready", "running", "interrupted", "waiting", "terminated"};
char *TYPE[] = {"regular", "producer", "consumer", "mutual_A", "mutual_B"};

/**
 * Returns a pcb pointer to heap allocation.
 * 
 * @return a pointer to a new PCB object
 */
PCB_p PCB_construct(int *ptr_error) {
    PCB_p this = (PCB_p) malloc(sizeof (struct PCB));
    this->regs = (REG_p) malloc(sizeof (union regfile));
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
        int j;
        this->reg.pc = DEFAULT_PC;
        this->reg.MAX_PC = rand() % (MAX_PC_RANGE + 1 - MAX_PC_MIN) + MAX_PC_MIN;
        this->reg.sw = DEFAULT_SW;
        this->reg.term_count = 0; //set to 0 for infinite process
        this->reg.TERMINATE = ((int)(rand() % 100 >= TERM_INFINITE_CHANCE)) * (rand() % TERM_RANGE + 1);
        for (t = 0; t < IO_NUMBER * IO_CALLS; t++)
            this->reg.IO_TRAPS[(int) (t / IO_CALLS)][t % IO_CALLS] = MIN_IO_CALL + (rand() % (this->reg.MAX_PC - MIN_IO_CALL));
        for (t = 0; t < IO_NUMBER * IO_CALLS; t++)
            for (j = 0; j < t; j++)
                if (this->reg.IO_TRAPS[(int) (j / IO_CALLS)][j % IO_CALLS] == 
                    this->reg.IO_TRAPS[(int) (t / IO_CALLS)][t % IO_CALLS])
                    this->reg.IO_TRAPS[(int) (j / IO_CALLS)][j % IO_CALLS] = -1; //duplicate values erased
    }
    return this;
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
    static int PRIORITIES[] = {PRIORITY_0_CHANCE, PRIORITY_1_CHANCE,
                               PRIORITY_2_CHANCE, PRIORITY_3_CHANCE,
                               PRIORITY_OTHER_CHANCE};
    static word pidCounter = ULONG_MAX;
    static int firstCall = 1;
    if (!firstCall) {
        srand(time(NULL) << 1);
        firstCall = 0;
    }
    int error = (this == NULL) * PCB_NULL_ERROR;
    int t;
    if (!error) {
        this->pid = ++pidCounter;
        this->io = true;
        this->type = regular;
        int chance = rand() % 100;
        int percent = 0;
        int priority;
        this->priority = -1;
        for (priority = 0; priority < PRIORITIES_TOTAL; priority++) {
            percent += PRIORITIES[min(priority, PRIORITY_UNIQUE_UPTO)];
            if (chance < percent) {
                this->priority = priority;
                break;
            }
            //printf("\npid: %lu chance: %d percent %d priority: %d\n", this->pid, chance, percent, priority);
        }
        if (this->priority < 0) error += PCB_PRIORITY_ERROR;
        this->state = DEFAULT_STATE;
        this->timeCreate = clock();
        this->timeTerminate = 0;
        REG_init(this->regs, &error);
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
int PCB_setPid(PCB_p this, word pid) {
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
word PCB_getPid(PCB_p this, int *ptr_error) {
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
int PCB_setPc(PCB_p this, word pc) {
    int error = (this == NULL) * PCB_NULL_ERROR;
    if (!error) {
        this->regs->reg.pc = pc;
    }
    return error; // TODO: write
}

/**
 * Returns the pc value state of the process.
 * 
 * @param this
 * @return the address where the program will resume
 */
word PCB_getPc(PCB_p this, int *ptr_error) {
    int error = (this == NULL) * PCB_NULL_ERROR;

    if (ptr_error != NULL) {
        *ptr_error += error;
    }
    return error ? -1 : this->regs->reg.pc; // TODO: write
}

/**
 * Sets the pid of the process.
 * 
 * @param this 
 * @param pid the new pid value
 * @return 
 */
int PCB_setSw(PCB_p this, word sw) {
    int error = (this == NULL) * PCB_NULL_ERROR;
    if (!error) {
        this->regs->reg.sw = sw;
    }
    return error; // TODO: write
}

/**
 * Returns the pc value state of the process.
 * 
 * @param this
 * @return the address where the program will resume
 */
word PCB_getSw(PCB_p this, int *ptr_error) {
    int error = (this == NULL) * PCB_NULL_ERROR;
    if (ptr_error != NULL) {
        *ptr_error += error;
    }
    return error ? ~0 : this->regs->reg.sw; // TODO: write
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
//        const char * format = "PID: 0x%04lx  PC: 0x%05lx  State: %s  Priority 0x%x";
//        snprintf(str, (size_t) PCB_TOSTRING_LEN - 1, format, this->pid, this->regs->reg.pc, STATE[this->state], this->priority);
        char regString[PCB_TOSTRING_LEN - 1];
        const char * format = "PID: 0x%04lx  PC: 0x%05lx  State: %s  Priority: 0x%x  Intensity: %s  Type: %s  %s";
        snprintf(str, (size_t) PCB_TOSTRING_LEN - 1, format, this->pid, this->regs->reg.pc, 
                 STATE[this->state], this->priority, this->io? "IO" : "CPU", TYPE[this->type],
                 Reg_File_toString(this->regs, regString, ptr_error));
    }

    if (ptr_error != NULL) {
        *ptr_error += error;
    }
    return str;

}

/**
 * Returns a string representing the contents of the regfile of the pcb. 
 * Note: parameter string must be 80 chars or more. 
 * @param this
 * @param str
 * @param int
 * @return string representing the contents of the regfile.
 */
char * Reg_File_toString(REG_p this, char *str, int *ptr_error) {
    int error = (this == NULL || str == NULL) * PCB_NULL_ERROR;
    if (!error) {
        str[0] = '\0';
        const char * format = "Max PC: 0x%05lx  Term Count: 0x%05lx  Terminate-on: 0x%05lx";
        snprintf(str, (size_t) PCB_TOSTRING_LEN - 1, format, this->reg.MAX_PC, 
                this->reg.term_count, this->reg.TERMINATE);
    }

    if (ptr_error != NULL) {
        *ptr_error += error;
    }
    return str;
}    