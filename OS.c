/*
 * Problem 3 - Discontinuities
 * TCSS 422 A Spring 2016
 * Bun Kak, Chris Ottersen, Mark Peters, Paul Zander
 */

#include "OS.h"

#define WRITE_TO_FILE false

//static const word int REGNUM = sizeof(struct regfile)/sizeof(word);

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
    
    if (DEBUG) printf("Main begin\n");
    
    if(WRITE_TO_FILE) {
        freopen("scheduleTrace.txt", "w", stdout);
    }
    
    int base_error = bootOS();
    if (DEBUG) printf("OS booted\n");
    
    mainLoopOS(&base_error);
    if (DEBUG) printf("OS shutdown\n");
            
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
    
    //idl pcb has special parameters
    idl = PCB_construct_init(&boot_error);
    idl->pid = ULONG_MAX;
    idl->priority = LOWEST_PRIORITY;
    idl->state = waiting;
    idl->timeCreate = 0;
    idl->timeTerminate = ULONG_MAX;
    
    idl->regs->reg.pc = 0;
    idl->regs->reg.MAX_PC = 1000;
    idl->regs->reg.sw = ULONG_MAX;
    idl->regs->reg.term_count = 0;
    idl->regs->reg.TERMINATE = 0;
    for (t = 0; t < IO_NUMBER * IO_CALLS; t++)
        idl->regs->reg.IO_TRAPS[t/IO_CALLS][t%IO_CALLS] = -1;
    
    
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
    
    if (DEBUG) printf("Main loop initialization\n");

    int t;
    word clock = 0;
    current = idl; //current's default state if no ready PCBs to run
    current->state = running;
    
    sysStackPush(current->regs, error);
    if (STACK_DEBUG) printf("current max_pc is %lu\n", current->regs->reg.MAX_PC);
    const CPU_p CPU = (CPU_p) malloc(sizeof(struct CPU));
    CPU->regs = (REG_p) malloc(sizeof (union regfile));
    REG_init(CPU->regs, error);
    if (STACK_DEBUG) printf("pre max_pc is %lu\n", CPU->regs->reg.MAX_PC);
    sysStackPop(CPU->regs, error);
    if (STACK_DEBUG) printf("post max_pc is %lu\n", CPU->regs->reg.MAX_PC);
    //    int r;
//        for (r = REGNUM-1; r >= 0; r--)
//            printf("at location %d: %lu\n", r, ((word*)&(CPU->regs))[r]);
    
    word * const pc = &(CPU->regs->reg.pc);
    word * const MAX_PC = &(CPU->regs->reg.MAX_PC);
    word * const sw = &(CPU->regs->reg.sw);
    word * const term_count = &(CPU->regs->reg.term_count);
    word * const TERMINATE = &(CPU->regs->reg.TERMINATE);
    word (* const IO_TRAPS)[IO_NUMBER][IO_CALLS] = &(CPU->regs->reg.IO_TRAPS);
    //last const IO_TRAPS mayyyy not be correct

    int exit = 0;
    
    if (*error) {
        printf("ERROR detected before launch! %d", *error);
        return *error;
    }
    
    if (DEBUG) printf("Main loop begin\n");

    /**************************************************************************/
    /*************************** MAIN LOOP OS *********************************/
    /**************************************************************************/
    do {
        clock++;
        exit = createPCBs(error);
        if (DEBUG) printf("PCBs created exit = %d\n", exit);
                
        if (current == NULL || error == NULL) {
            *error += CPU_NULL_ERROR;
            printf("ERROR current process unassigned or error lost! %d", *error);
            
        } else {

            /*** INCREMENT PC ***/
            (*pc)++;
            if (DEBUG) printf("PC incremented to %lu\n", *pc);
            
            /*** TERMINATE CHECK ***/
            if (*pc == *MAX_PC) {
                *pc -= (*pc);
                (*term_count)++;
                if (DEBUG) printf("term_count now %lu / %lu\n", *term_count, *TERMINATE);
                if (*term_count == *TERMINATE) {
                    word pid = current->pid;
                    if (DEBUG) printf("At cycle PC = %lu, terminate interrupts %lu\n", *pc, current->pid);
                    sysStackPush(CPU->regs, error);
                    trap_terminate();
                    sysStackPop(CPU->regs, error);
                    if (DEBUG) printf("Process %lu terminated\n", pid);
                    if (DEBUG) printf("At cycle PC = %lu, process %lu begins\n", *pc, current->pid);
                }
            }
            
            /*** TIMER CHECK ***/
            if (pthread_mutex_trylock(&MUTEX_timer)) {
                if (DEBUG) printf("Timer check \n");
                bool context_switch = false;
                if (INTERRUPT_timer) {
                    INTERRUPT_timer = false;
                    context_switch = true;
                }
                pthread_mutex_unlock(&MUTEX_timer);
                if (context_switch || PCB_SCHEDULE_EVERY) {
                    if (DEBUG) printf("At cycle PC = %lu, timer interrupts %lu\n", *pc, current->pid);
                    sysStackPush(CPU->regs, error);
                    interrupt(INTERRUPT_TIMER, NULL, error);
                    sysStackPop(CPU->regs, error);
                    if (DEBUG) printf("At cycle PC = %lu, process %lu begins\n", *pc, current->pid);
                }
                if (DEBUG) printf("No cycle PC \n");
            }
            
            /*** IO CHECK ***/
            if (DEBUG) printf("swretsdfgsdfg \n");
            for (t = 0; t < IO_NUMBER; t++)
                if (!pthread_mutex_trylock(&(IO[t]->MUTEX_io))) {
                    if (DEBUG) printf("mut pre %d\n", t);
                    if (IO[t]->INTERRUPT_iocomplete)
                        interrupt(INTERRUPT_IOCOMPLETE, (void*)t, error);
                    pthread_mutex_unlock(&(IO[t]->MUTEX_io));
                    if (DEBUG) printf("mut pst %d\n", t);
                }
            
            /*** PCB TRAPS CHECK ***/
            for (t = 0; t < IO_NUMBER * IO_CALLS; t++) 
//                printf("---------------------%d: %lu\n", t, (*IO_TRAPS)[t/IO_CALLS][t%IO_CALLS]);   
                if (*pc == (*IO_TRAPS)[t/IO_CALLS][t%IO_CALLS]) {
                    if (DEBUG) printf("IO %d at PC %lu for process %lu\n", t/IO_CALLS, *pc, current->pid);
                    sysStackPush(CPU->regs, error);
                    trap_iohandler(t/IO_CALLS, error);
                    sysStackPop(CPU->regs, error);
                    if (DEBUG) printf("Process %lu at PC %lu place in wQ of IO %d\n", current->pid, *pc, t/IO_CALLS);
                    if (DEBUG) printf("At cycle PC = %lu, process %lu begins\n", *pc, current->pid);   
                }
                
            if (DEBUG) printf("PC cycle %lu finished\n", *pc);

        }   

    } while (!*error  && !exit);
    
    sysStackPush(CPU->regs, error);
    sysStackPop(current->regs, error);
    
    /**************************************************************************/
    /*************************** *********** **********************************/
    /**************************************************************************/    
    if (EXIT_STATUS_MESSAGE) printf("System Clock: %lu\n", clock);
    
    cleanup(error);
    
    return *error;
}

/******************************************************************************/
/******************************* THREADS **************************************/
/******************************************************************************/


void* timer(void* unused) {
    printf("begin TIMER THREAD\n");
}

void* io(void* tid) {
    //tid is int for thread number
    printf("begin IO %d THREAD\n", (int)tid);
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
            isr_timer(error);
            break;

        case INTERRUPT_IOCOMPLETE:
            isr_iocomplete(((int)args), error);
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
    scheduler(error);
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
void scheduler(int* error) {

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
            
    if (readyQ->size < 2) {
        PCB_setState(current, running);
        sysStackPush(current->regs, error);
        return;
    }
    
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
        
        
        /**** some point after here is the mysterious system crash on idl switch*/
        
        
        char rdqstr[PCB_TOSTRING_LEN];
        printf(">Returned to ready queue: %s\n", PCB_toString(readyQ->tail->data, rdqstr, error));        
        int stz = FIFOQ_TOSTRING_MAX;
        char str[stz];
        printf(">%s\n", FIFOq_toString(readyQ, str, &stz, error));

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
    static bool first_batch = !PCB_CREATE_FIRST;
    if (first_batch && (!PCB_CREATE_EVERY) && (rand() % TIME_QUANTUM))
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
    if (!first_batch) first_batch = r;
    if (DEBUG) printf("total created pcbs: %d and exit = %d\n", processes_created, (processes_created >= MAX_PROCESSES ? -1 : 0));
    return (processes_created >= MAX_PROCESSES ? -1 : 0);
}

/******************************************************************************/
/************************** SYSTEM SUBROUTINES ********************************/
/******************************************************************************/

int sysStackPush(REG_p fromRegs, int* error) {
    int r;
    
    for (r = 0; r < REGNUM; r++)
        if (SysPointer >= SYSSIZE-1) {
            *error += CPU_STACK_ERROR;
            printf("ERROR: Sysstack exceeded\n");        
        } else
            SysStack[SysPointer++] = fromRegs->gpu[r];

    int i;
    if (STACK_DEBUG) 
        for (i = 0; i <= SysPointer; i++) {
            printf("\tPostPush SysStack[%d] = %lu\n", i, SysStack[i]);
        }
}

int sysStackPop(REG_p toRegs, int* error) {
    int r;
    if (STACK_DEBUG) printf("max_pc is %lu\n", toRegs->reg.MAX_PC);
    for (r = REGNUM-1; r >= 0; r--)
        if (SysPointer <= 0) {
            *error += CPU_STACK_ERROR;
            printf("ERROR: Sysstack exceeded\n");
            toRegs->gpu[r] = STACK_ERROR_DEFAULT;
        } else {
            if (STACK_DEBUG) printf("\tpopping SysStack[%d] = %lu ", SysPointer-1, SysStack[SysPointer-1]);
            toRegs->gpu[r] = SysStack[--SysPointer];
            if (STACK_DEBUG) printf("at location %d: %lu\n", r, toRegs->gpu[r]);
        }
    if (STACK_DEBUG) printf("max_pc is %lu\n", toRegs->reg.MAX_PC);
    int i;
    for (i = 0; i <= SysPointer; i++) {
        if (STACK_DEBUG) printf("\tPost-Pop SysStack[%d] = %lu\n", i, SysStack[i]);
    }
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
    for (t = 0; t < IO_NUMBER; t++) {
        pthread_mutex_lock(&(IO[t]->MUTEX_io));
        wQ[9] = (char)t;
        queueCleanup(IO[t]->waitingQ, wQ, error);
        pthread_mutex_destroy(&(IO[t]->MUTEX_io));
        free(IO[t]);
        IO[t] = NULL;
        pthread_mutex_unlock(&(IO[t]->MUTEX_io));
    }
            
    int endidl = current->pid == idl->pid;
//    printf("%lu %lu\n", current->pid, idl->pid);
    if (idl != NULL) {
        if (EXIT_STATUS_MESSAGE) {
            char pcbstr[PCB_TOSTRING_LEN];
            printf("System Idle: %s\n", PCB_toString(idl, pcbstr, error));
        }
        PCB_destruct(idl);
    }
    if (!endidl && current != NULL) {
        if (EXIT_STATUS_MESSAGE) {
            char pcbstr[PCB_TOSTRING_LEN];
            printf("Running PCB: %s\n", PCB_toString(current, pcbstr, error));
        }
        PCB_destruct(current);
    }
    
}

/* Deallocates the queue data structures on the C simulator running this program
 * so that we don't have memory leaks and horrible garbage.
 */
void queueCleanup(FIFOq_p queue, char *qstr, int *error) {
    int stz = FIFOQ_TOSTRING_MAX;
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
