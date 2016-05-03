/*
 * Problem 3 - Discontinuities
 * TCSS 422 A Spring 2016
 * Bun Kak, Chris Ottersen, Mark Peters, Paul Zander
 */

#include "OS.h"

#define WRITE_TO_FILE false

static const word int REGNUM = sizeof(struct regfile)/sizeof(word);

/*global declarations for system stack*/
static word SysStack[SYSSIZE];
static int SysPointer;


/*timer fields*/
static thread THREAD_timer;
static mutex MUTEX_timer;
static bool INTERRUPT_timer;

/*IO*/
static io_thread IO[IO_NUMBER];

/*OS only declarations for current and idle processes*/
static PCB_p current;
static PCB_p idl;
static FIFOq_p createQ;
static FIFOq_p readyQ;
static FIFOq_p terminateQ;


/* Launches the OS. Sets default values, initializes idle process and calls the
 * mainLoopOS to simulate running the cpu. Afterwards it cleans up reports
 * any errors encountered.
 */
int main(void) {
    
    if(WRITE_TO_FILE) {
        freopen("scheduleTrace.txt", "w", stdout);
    }
    
    int base_error = bootOS();
    
    mainLoopOS(&base_error);
            
    stackCleanup();
    
    if (base_error) {
        if (EXIT_STATUS_MESSAGE) printf("System exited with error %d\n", base_error);
        if (OUTPUT) printf("\nExit due to ERROR; see output\n");
    } else {
        if (EXIT_STATUS_MESSAGE) printf("System exited without incident\n");
        if (OUTPUT) printf("\n%d processes have been created so system has exited\n", MAX_PROCESSES);
    }

    return base_error;

}

//initializes values and returns error
int bootOS() {

    int boot_error = 0;
    int t;

    //system wide
    srand(time(NULL)); // seed random with current time
    SysPointer = 0; //points at next unassigned stack item; 0 is empty
    
    //Timer
    INTERRUPT_timer = false;
    pthread_mutex_init(&MUTEX_timer, NULL);
    pthread_create(&THREAD_timer, NULL, timer, NULL);
        
    //IO
    for (t = 0; t < IO_NUMBER; t++) {
        IO[t] = (io_thread)malloc(sizeof(struct io_thread_type));
        IO[t]->waitingQ = FIFOq_construct(&boot_error);
        IO[t]->INTERRUPT_iocomplete = false;
        pthread_mutex_init(&(IO[t]->MUTEX_io), NULL);
        pthread_create(&(IO[t]->THREAD_io), NULL, io, (void*)t);
    }
    
    //idl pcb
    idl = PCB_construct_init(&boot_error);
    idl->pid = ULONG_MAX;
    idl->priority = LOWEST_PRIORITY;
    idl->regs->sw = ULONG_MAX;    
    
    //queues
    createQ = FIFOq_construct(&boot_error);
    readyQ = FIFOq_construct(&boot_error);
    terminateQ = FIFOq_construct(&boot_error);
    
    return boot_error;
}

/* Main loop for the operating system. Initializes queues and PC/SW values.
 * Runs until exit is triggered (in assignment 2, this is when 30 PCBs have been
 * created). Repeatedly creates new PCBs, simulates running, gets interrupted
 * by the timer, and restarting the loop.
 * 
 * returns error
 */
int mainLoopOS(int *error) {

    int t;
    
    current = idl; //current's default state if no ready PCBs to run
    current->state = running;
    
    sysStackPush(current->regs, error);
    const CPU_p CPU = (CPU_p) malloc(sizeof(struct CPU));
    sysStackPop(CPU->regs, error);
    
    word * const pc = &(CPU->regs->pc);
    word * const MAX_PC = &(CPU->regs->MAX_PC);
    word * const sw = &(CPU->regs->sw);
    word * const term_count = &(CPU->regs->term_count);
    word * const TERMINATE = &(CPU->regs->TERMINATE);
    word (* const IO_TRAPS)[IO_NUMBER][IO_CALLS] = &(CPU->regs->IO_TRAPS);
    //last const IO_TRAPS mayyyy not be correct
    
    int exit = 0;
    
    if (*error) {
        printf("ERROR detected before launch! %d", *error);
        return *error;
    }

    /**************************************************************************/
    /*************************** MAIN LOOP OS *********************************/
    /**************************************************************************/
    do {
        
        exit = createPCBs(error);
                
        if (current == NULL || error == NULL) {
            *error += CPU_NULL_ERROR;
            printf("ERROR current process unassigned or error lost! %d", *error);
            
        } else {
            
            /*** INCREMENT PC ***/
            (*pc)++;
            
            /*** TERMINATE CHECK ***/
            if (*pc == *MAX_PC) {
                *pc -= (*pc);
                (*term_count)++;
                if (*term_count == *TERMINATE) {
                    sysStackPush(CPU->regs, error);
                    trap_terminate();
                    sysStackPop(CPU->regs, error);
                }
            }
            
            /*** TIMER CHECK ***/
            if (pthread_mutex_trylock(&MUTEX_timer)) {
                pthread_mutex_lock(&MUTEX_timer);
                bool context_switch = false;
                if (INTERRUPT_timer) {
                    INTERRUPT_timer = false;
                    context_switch = true;
                }
                pthread_mutex_unlock(&MUTEX_timer);
                if (context_switch) {
                    sysStackPush(CPU->regs, error);
                    interrupt(INTERRUPT_TIMER, NULL, error);
                    sysStackPop(CPU->regs, error);
                }
                
            }
            
            /*** IO CHECK ***/
            for (t = 0; t < IO_NUMBER; t++)
                if (pthread_mutex_trylock(&(IO[t]->MUTEX_io))) {
                    pthread_mutex_lock(&(IO[t]->MUTEX_io));
                    if (IO[t]->INTERRUPT_iocomplete)
                        interrupt(INTERRUPT_IOCOMPLETE, (void*)t, error);
                        //isr_iocomplete(t, error);
                    pthread_mutex_unlock(&(IO[t]->MUTEX_io));
                }
            
            /*** PCB TRAPS CHECK ***/
            for (t = 0; t < IO_NUMBER * IO_CALLS; t++)
                if (*pc == *(IO_TRAPS[t/IO_CALLS][t%IO_CALLS])) {
                    sysStackPush(CPU->regs, error);
                    trap_iohandler(t/IO_CALLS, error);
                    sysStackPop(CPU->regs, error);
                }


        }   
        
    } while (!*error  && !exit);
    /**************************************************************************/
    /*************************** *********** **********************************/
    /**************************************************************************/    
    
    cleanup(error);
    
    return *error;
}

/******************************************************************************/
/******************************* THREADS **************************************/
/******************************************************************************/


void* timer(void* unused) {
    
}

void* io(void* tid) {
    //tid is int for thread number
}

/******************************************************************************/
/******************************** TRAPS ***************************************/
/******************************************************************************/

void    trap_terminate() {
    
}
void trap_iohandler(const int T, int* error) {
    
}

/******************************************************************************/
/*********************** INTERRUPT SERVICE ROUTINES ***************************/
/******************************************************************************/

void interrupt(const int INTERRUPT, void* args, int* error) {
    switch(INTERRUPT) {
        
        case NO_INTERRUPT:
            current->state = running;
            break;

        case INTERRUPT_TIMER:
            // state remains unchanged
            isr_timer(error);
            break;

        case INTERRUPT_IOCOMPLETE:
            isr_iocomplete(((int*)args), error);
            break;
    }
}

/* Interrupt service routine for the timer: interrupts the current PCB and saves
 * the CPU state to it before calling the scheduler.
 */
void isr_timer(int* error) {
    //change the state from running to interrupted
    PCB_setState(current, interrupted);
    
    //assigns Current PCB PC and SW values to popped values of SystemStack
    if (DEBUG) printf("\t\tStack going to pop isrtimer: %d\n", SysPointer);
    sysStackPop(current->regs, error);
       
    //call Scheduler and pass timer interrupt parameter
    scheduler(INTERRUPT_TIMER, error);
}

void isr_iocomplete(const int IO, int* error) {
    
}

/******************************************************************************/
/************************** SCHEDULERS/LOADERS ********************************/
/******************************************************************************/

/* Always schedules any newly created PCBs into the ready queue, then checks
 * the interrupt type: if the interrupt is a timer interrupt, requeues the
 * currently running process in the ready queue and sets its state to ready.
 * Then calls the dispatcher.
 */
void scheduler(const int INTERRUPT, int* error) {

    //for measuring every 4th output
    static int context_switch = 0;

    PCB_p temp;
                            
    if (createQ == NULL) {
	*error += FIFO_NULL_ERROR;
	printf("%s", "ERROR: createQ is null");
	return;
    }

    if (readyQ == NULL) {
	*error += FIFO_NULL_ERROR;
	printf("%s", "ERROR: readyQ is null");
	return;
    }
    
    //enqueue any created processes
    while (!FIFOq_is_empty(createQ, error)) {
        temp = FIFOq_dequeue(createQ, error);
        temp->state = ready;
        FIFOq_enqueuePCB(readyQ, temp, error);
        if (OUTPUT) {
            char pcbstr[PCB_TOSTRING_LEN];
            printf(">Enqueued to ready queue: %s\n", PCB_toString(temp, pcbstr, error));
        }
    }

    if (DEBUG) printf("createQ transferred to readyQ\n");
		
    //reroute based on interrupt
    switch (INTERRUPT) {

        case NO_INTERRUPT:
            if (current != idl && current->state != terminated) {
                current->state = ready;
                FIFOq_enqueuePCB(readyQ, current, error);
            } else idl->state = waiting;
            dispatcher(error);
            break;

        case INTERRUPT_TIMER:
            context_switch++;
            
            if (!(context_switch % 4)) {
                char pcbstr[PCB_TOSTRING_LEN];
                printf(">PCB: %s\n", PCB_toString(current, pcbstr, error));        
                char rdqstr[PCB_TOSTRING_LEN];
                printf(">Switching to: %s\n", PCB_toString(readyQ->head->data, rdqstr, error));        
            }
            
            
            if (current != idl && current->state != terminated) {
                current->state = ready;
                FIFOq_enqueuePCB(readyQ, current, error);
            } else idl->state = waiting;
            dispatcher(error);

            if (!(context_switch % 4)) {
                char runstr[PCB_TOSTRING_LEN];
                printf(">Now running: %s\n", PCB_toString(current, runstr, error));        
                char rdqstr[PCB_TOSTRING_LEN];
                printf(">Returned to ready queue: %s\n", PCB_toString(readyQ->tail->data, rdqstr, error));        
                int stz = 1024;
                char str[stz];
                printf(">%s\n", FIFOq_toString(readyQ, str, &stz, error));

            }

            break;

        case INTERRUPT_IOCOMPLETE:
            current->state = waiting;
            //IO stuffb
            dispatcher(error);
            break;
             
    }

    //"housekeeping"

}

/* Dispatches a new current process by dequeuing the head of the ready queue,
 * setting its state to running and copying its CPU state onto the stack.
 */
void dispatcher(int* error) {

    if (readyQ == NULL) {
	*error += FIFO_NULL_ERROR;
	printf("%s", "ERROR: readyQ is null");
    } else if (!FIFOq_is_empty(readyQ, error)) {	//dequeue the head of readyQueue
        current = FIFOq_dequeue(readyQ, error);
    } else {
        current = idl;
    }

    //change current's state to running point
    current->state = running;
    
    if (DEBUG) {
        char pcbstr[PCB_TOSTRING_LEN];
        printf("CURRENT: \t%s\n", PCB_toString(current, pcbstr, error));        
    }
    
    //copy currents's PC value to SystemStack
    if (DEBUG) printf("\t\tStack going to push dispatch: %d\n", SysPointer);
    sysStackPush(current->regs, error);

}

/* Creates 0 to 5 PCBs and enqueues them into a special queue, create queue.
 * Keeps track of how many PCBs have been created and sends an exit signal
 * when 30 have been created.
 */
int createPCBs(int *error) {
    if (rand() % TIME_QUANTUM)
        return 0;
    int i;
    // random number of new processes between 0 and 5
    int r = rand() % (MAX_NEW_PCB+1);
    char buffer[PCB_TOSTRING_LEN];
    static int processes_created = 0;
    
    if (r + processes_created >= MAX_PROCESSES) {
        r = MAX_PROCESSES - processes_created;
    }
    
    if (START_IDLE && processes_created == 0) {
        r = 0;
        processes_created += 1;
    }

    if (createQ == NULL) {
        *error += CPU_NULL_ERROR;
        printf("ERROR: FIFOq_p passed to createPCBs is NULL\n");
        return *error;
    }
    
    if (DEBUG) printf("createPCBs: creating %d PCBs and enqueueing them to createQ\n", r);
    for (i = 0; i < r; i++) {
        // PCB_construct initializes state to 0 (created)
        PCB_p newPcb = PCB_construct_init(error);
        if (DEBUG) printf("New PCB created: %s\n", PCB_toString(newPcb, buffer, error));
        FIFOq_enqueuePCB(createQ, newPcb, error);
        processes_created++;
    }
    if (DEBUG) printf("PCBs all created\n");
    return processes_created >= MAX_PROCESSES ? -1 : 0;
}

/******************************************************************************/
/************************** SYSTEM SUBROUTINES ********************************/
/******************************************************************************/

int sysStackPush(REG_p fromRegs, int* error) {
    int r;
    
    for (r = 0; r < REGNUM; r++)
        if (SysPointer >= SYSSIZE) {
            *error += CPU_STACK_ERROR;
            printf("ERROR: Sysstack exceeded\n");        
        } else
            SysStack[SysPointer++] = ((word*)fromRegs)[r];
        //these are the items that need to pop off the stack
//    word pc = current->regs->pc; //we used a long to match the PCB value for PC
//    word MAX_PC = current->regs->MAX_PC;
//    word sw = current->regs->sw;
//    word term_count = current->regs->term_count;
//    word TERMINATE = current->regs->TERMINATE;
//    word IO_TRAPS[IO_NUMBER][IO_CALLS];
//    for (t = 0; t < IO_NUMBER * IO_CALLS; t++)
//       IO_TRAPS[(int)(t/IO_CALLS)][t%IO_CALLS] = current->IO_TRAPS[(int)(t/IO_CALLS)][t%IO_CALLS];

}

int sysStackPop(REG_p toRegs, int* error) {
    int r;
    for (r = REGNUM-1; r >= 0; r--)
        if (SysPointer < 0) {
            *error += CPU_STACK_ERROR;
            printf("ERROR: Sysstack exceeded\n");
            ((word*)&toRegs)[r] = STACK_ERROR_DEFAULT;
        } else
            ((word*)&toRegs)[r] = SysStack[SysPointer--];  
}

/******************************************************************************/
/**************************** DEALLOCATION ************************************/
/******************************************************************************/

void cleanup(int* error) {

    int t;
    
    queueCleanup(terminateQ, "terminateQ", error);
    queueCleanup(readyQ, "readyQ", error);
    queueCleanup(createQ, "createQ", error);
    
    pthread_mutex_destroy(&MUTEX_timer);
    char wQ[10] = "waitingQ_x";
    for (t = 0; t < IO_NUMBER; t++)
        if (pthread_mutex_trylock(&(IO[t]->MUTEX_io))) {
            wQ[9] = (char)t;
            queueCleanup(IO[t]->waitingQ, wQ, error);
            pthread_mutex_destroy(&(IO[t]->MUTEX_io));
            free(IO[t]);
            IO[t] = NULL;
        }
            

    if (current != idl) {
        if (EXIT_STATUS_MESSAGE) {
            char pcbstr[PCB_TOSTRING_LEN];
            printf("System Idle: %s\n", PCB_toString(idl, pcbstr, error));
        }
        PCB_destruct(idl);
    }
    if (current != NULL) {
        if (EXIT_STATUS_MESSAGE) {
            char pcbstr[PCB_TOSTRING_LEN];
            printf("Running PCB: %s\n", PCB_toString(idl, pcbstr, error));
        }
        PCB_destruct(current);
    }
    
}

/* Deallocates the queue data structures on the C simulator running this program
 * so that we don't have memory leaks and horrible garbage.
 */
void queueCleanup(FIFOq_p queue, char *qstr, int *error) {
    int stz = 256;
    char str[stz];
    
    if (!FIFOq_is_empty(queue, error)) {
        if (EXIT_STATUS_MESSAGE) {
            printf("System exited with non-empty queue %s\n", qstr);
            printf("\t%s\n", FIFOq_toString(queue, str, &stz, error));
        }
        while (!FIFOq_is_empty(queue, error)) {
            PCB_p pcb = FIFOq_dequeue(queue, error);
            if (pcb != idl) {
                char pcbstr[PCB_TOSTRING_LEN];
                if (EXIT_STATUS_MESSAGE) printf("\t%s\n", PCB_toString(pcb, pcbstr, error));
                PCB_destruct(pcb);
        
            }
    
        }
    }
    FIFOq_destruct(queue, error);
}

/* Deallocates the stack data structure on the C simulator running this program
 * so that we don't have memory leaks and horrible garbage.
 */
void stackCleanup() {
    int i;
    if (SysPointer > 0) {
        if (EXIT_STATUS_MESSAGE) {
            printf("System exited with non-empty stack\n");
            for (i = 0; i <= SysPointer; i++) {
                printf("\tSysStack[%d] = %lu\n", i, SysStack[i]);
            }
        }
    }
}
