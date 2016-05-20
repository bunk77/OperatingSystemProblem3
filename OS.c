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
static int closeable;

/*timer fields*/
static thread THREAD_timer;
static mutex MUTEX_timer;
static cond COND_timer;
static bool INTERRUPT_timer;
static bool SHUTOFF_timer;
static word clock_;

/*IO*/
static io_thread IO[IO_NUMBER];

/*OS only declarations for current and idle processes*/
static PCB_p current;
static PCB_p idl;
static FIFOq_p createQ;
static FIFOq_p readyQ[PRIORITIES_TOTAL];
static FIFOq_p terminateQ;

/* Launches the OS. Sets default values, initializes idle process and calls the
 * mainLoopOS to simulate running the cpu. Afterwards it cleans up reports
 * any errors encountered.
 */
int main(void) {

    if (DEBUG) printf("Main begin\n");

//    nanosleeptest();
    
    if (WRITE_TO_FILE) {
        freopen("scheduleTrace.txt", "w", stdout);
    }

    int base_error = bootOS();
    if (DEBUG) printf("OS booted\n");

    mainLoopOS(&base_error);
    if (DEBUG) printf("OS shutdown\n");

    stackCleanup();

    if (base_error) {
        if (EXIT_STATUS_MESSAGE || OUTPUT) printf("\n>System exited with error %d\n", base_error);
    } else {
        if (EXIT_STATUS_MESSAGE) printf("\n>System exited without incident\n");
        if (OUTPUT)
            if (EXIT_ON_MAX_PROCESSES) printf("\n>%d processes have been created so system has exited\n", MAX_PROCESSES);
            else printf("\n>Of %d processes created, all terminable ones have terminated so system has exited\n", MAX_PROCESSES);
    }

    return base_error;

}

//initializes values and returns error

int bootOS() {

    int boot_error = OS_NO_ERROR;
    int t;

    //system wide
    srand(time(NULL)); // seed random with current time
    SysPointer = 0; //points at next unassigned stack item; 0 is empty
    closeable = 0;
    
    //Timer
    clock_ = 0;
    INTERRUPT_timer = false;
    SHUTOFF_timer = false;
    pthread_mutex_init(&MUTEX_timer, NULL);
    pthread_cond_init(&COND_timer, NULL);
    pthread_create(&THREAD_timer, NULL, timer, NULL);

    //IO
    for (t = 0; t < IO_NUMBER; t++) {
        IO[t] = (io_thread) malloc(sizeof (struct io_thread_type));
        IO[t]->waitingQ = FIFOq_construct(&boot_error);
        IO[t]->INTERRUPT_iocomplete = false;
        IO[t]->SHUTOFF_io = false;
        pthread_mutex_init(&(IO[t]->MUTEX_io), NULL);
        pthread_cond_init(&(IO[t]->COND_io), NULL);
        pthread_create(&(IO[t]->THREAD_io), NULL, io, (void*) t);
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
        idl->regs->reg.IO_TRAPS[t / IO_CALLS][t % IO_CALLS] = -1;


    //queues
    createQ = FIFOq_construct(&boot_error);
    for (t = 0; t < PRIORITIES_TOTAL; t++)
        readyQ[t] = FIFOq_construct(&boot_error);
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
    bool exit_okay = false;
    current = idl; //current's default state if no ready PCBs to run
    current->state = running;

    sysStackPush(current->regs, error);
    if (STACK_DEBUG) printf("current max_pc is %lu\n", current->regs->reg.MAX_PC);
    const CPU_p CPU = (CPU_p) malloc(sizeof (struct CPU));
    CPU->regs = (REG_p) malloc(sizeof (union regfile));
    REG_init(CPU->regs, error);
    if (STACK_DEBUG) printf("pre max_pc is %lu\n", CPU->regs->reg.MAX_PC);
    sysStackPop(CPU->regs, error);
    if (STACK_DEBUG) printf("post max_pc is %lu\n", CPU->regs->reg.MAX_PC);

    word * const pc = &(CPU->regs->reg.pc);
    word * const MAX_PC = &(CPU->regs->reg.MAX_PC);
    word * const sw = &(CPU->regs->reg.sw);
    word * const term_count = &(CPU->regs->reg.term_count);
    word * const TERMINATE = &(CPU->regs->reg.TERMINATE);
    word(* const IO_TRAPS)[IO_NUMBER][IO_CALLS] = &(CPU->regs->reg.IO_TRAPS);

    int exit = 0;

    if (*error) {
        printf("ERROR detected before launch! %d", *error);
        return *error;
    }

    if (OUTPUT) printf(">OS System Clock at Start %lu\n\n", clock_);
    if (DEBUG) printf("Main loop begin\n");
    

    /**************************************************************************/
    /*************************** MAIN LOOP OS *********************************/
    /**************************************************************************/
    do {
        clock_++;
        exit = createPCBs(error);
        if (!EXIT_ON_MAX_PROCESSES && exit && !exit_okay) {
            exit_okay = true;
            exit = 0;
        }
        
        if (DEBUG) printf("PCBs created exit = %d\n", exit);

        if (current == NULL || error == NULL) {
            if (error != NULL)
                *error += CPU_NULL_ERROR;
            exit = -1;
            printf("ERROR current process unassigned or error lost! %d", *error);

        } else {

            /*** INCREMENT PC ***/
            (*pc)++;
            if (DEBUG) printf("PCB %lu PC incremented to %lu of %lu\n", current->pid, *pc, *MAX_PC);

            /*** TERMINATE CHECK ***/
            if (*pc == *MAX_PC) {
                *pc -= (*pc);
                (*term_count)++;
                if (DEBUG) printf("term_count now %lu / %lu\n", *term_count, *TERMINATE);
                if (*term_count == *TERMINATE) {
                    word pid = current->pid;
                    if (DEBUG) printf("At cycle PC = %lu, terminate interrupts %lu\n", *pc, current->pid);
                    sysStackPush(CPU->regs, error);
                    trap_terminate(error);
                    sysStackPop(CPU->regs, error);
                    if (DEBUG) printf("Process %lu terminated\n", pid);
                    if (DEBUG) printf("At cycle PC = %lu, process %lu begins\n", *pc, current->pid);
                }
            }
            if (current != idl && current->pid > MAX_PROCESSES) {
                if (DEBUG) printf("post-term: pcb with pid %lu exceeds max processes, exiting system\n", current->pid);
                break;
            }
            
            /*** TIMER CHECK ***/
            if (!pthread_mutex_trylock(&MUTEX_timer)) { 
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
                    continue;
                }
                if (DEBUG) printf("No cycle PC \n");
            }
            if (current != idl && current->pid > MAX_PROCESSES) {
                if (DEBUG) printf("post-time: pcb with pid %lu exceeds max processes, exiting system\n", current->pid);
                break;
            }

            /*** IO CHECK ***/
            if (DEBUG) printf("Checking IO if complete \n");
            for (t = 0; t < IO_NUMBER; t++)
                if (!pthread_mutex_trylock(&(IO[t]->MUTEX_io))) {
                    if (DEBUG) printf("io %d Mutex locked\n", t + FIRST_IO);
                    if (IO[t]->INTERRUPT_iocomplete) {
                        sysStackPush(CPU->regs, error);
                        interrupt(INTERRUPT_IOCOMPLETE, (void*) t, error);
                        sysStackPop(CPU->regs, error);
                        IO[t]->INTERRUPT_iocomplete = false;
                    }
                    pthread_mutex_unlock(&(IO[t]->MUTEX_io));
                    pthread_cond_signal(&(IO[t]->COND_io));
                    if (DEBUG) printf("io %d Mutex unlocked\n", t + FIRST_IO);
                }
            if (current != idl && current->pid > MAX_PROCESSES) {
                if (DEBUG) printf("post-iocm: pcb with pid %lu exceeds max processes, exiting system\n", current->pid);
                break;
            }
            
            /*** PCB TRAPS CHECK ***/
            for (t = 0; t < IO_NUMBER * IO_CALLS; t++)
                 if (*pc == (*IO_TRAPS)[t / IO_CALLS][t % IO_CALLS]) {
                    t = t / IO_CALLS;
                    if (DEBUG) printf("Process %lu at PC %lu place in wQ of IO %d\n", current->pid, *pc, t + FIRST_IO);
                    pthread_mutex_lock(&(IO[t]->MUTEX_io));
                    
                    sysStackPush(CPU->regs, error);
                    trap_iohandler(t, error);
                    sysStackPop(CPU->regs, error);
                    pthread_mutex_unlock(&(IO[t]->MUTEX_io));
                    pthread_cond_signal(&(IO[t]->COND_io));
                    if (DEBUG) printf("At cycle PC = %lu, process %lu begins\n", *pc, current->pid);
                    break;
                } 
            if (current != idl && current->pid > MAX_PROCESSES) {
                if (EXIT_STATUS_MESSAGE) printf("post-trap: pcb with pid %lu exceeds max processes, exiting system\n", current->pid);
                break;
            }
            
            if (DEBUG) printf("PCB %lu PC cycle %lu finished\n", current->pid, *pc);

        }
        
        if (!EXIT_ON_MAX_PROCESSES)
            if (exit_okay && closeable == 0)
                exit = -1;
            else exit = 0;
        
    } while (!*error && !exit);
    /**************************************************************************/
    /*************************** *********** **********************************/
    /**************************************************************************/
    if (DEBUG) printf("Main loop OS stop\n");

    sysStackPush(CPU->regs, error);
    sysStackPop(current->regs, error);

    if (EXIT_STATUS_MESSAGE) printf("\nOS System Clock at Exit: %lu\n", clock_);

//    for(t = 0; t < TIMER_SLEEP; t++);
    
    cleanup(error);

    return *error;
}

/******************************************************************************/
/******************************* THREADS **************************************/
/******************************************************************************/


void* timer(void* unused) {

    if (THREAD_DEBUG) printf("\tTIMER: begin THREAD\n");

    int c;
    word next = 0;
    bool shutoff;
    word clock = 0;
    
    do {

        do {
            for(c = 0; c < TIMER_SLEEP; c++); //sleeping simulation
            pthread_mutex_lock(&MUTEX_timer);
            clock = clock_;
            shutoff = SHUTOFF_timer;
            pthread_mutex_unlock(&MUTEX_timer);
            pthread_cond_signal(&COND_timer);
            if (THREAD_DEBUG) printf("\tTIMER: clock is %lu out of %lu, shutoff is %s\n", clock, next, shutoff ? "true" : "false");
        } while (clock < next && !shutoff);
        pthread_mutex_lock(&MUTEX_timer);
        INTERRUPT_timer = true;
        next = clock_ + TIME_QUANTUM;      
        if (THREAD_DEBUG) printf("\tTIMER: begin clock at %lu\n", clock);  
        pthread_mutex_unlock(&MUTEX_timer);
    } while (!shutoff);
    
    if (THREAD_DEBUG) printf("\tTIMER: end clock at %lu\n", clock);
    pthread_mutex_unlock(&MUTEX_timer);
    pthread_cond_signal(&COND_timer);
    pthread_exit(NULL);
    return NULL;
}

void* io(void* tid) {
    
    int t = (int) tid;
    //tid is int for thread number
    
    if (THREAD_DEBUG) printf("\t\tIO %d: begin THREAD\n", t + FIRST_IO);

    int io_error = OS_NO_ERROR;
    int c;
    bool empty;
    bool shutoff;
    
    
    do {
        pthread_mutex_lock(&(IO[t]->MUTEX_io));
        shutoff = IO[t]->SHUTOFF_io;
        empty = FIFOq_is_empty(IO[t]->waitingQ, &io_error);
        pthread_mutex_unlock(&(IO[t]->MUTEX_io));
        while (!empty && !shutoff) {
            if (THREAD_DEBUG) printf("\t\tIO %d: queue has %d PCBs left; beginning IO ops\n", t + FIRST_IO, IO[t]->waitingQ->size);
            word sleep = rand() % (IO_MAX_SLEEP - IO_MIN_SLEEP) + IO_MIN_SLEEP;
            for(c = 0; c < sleep; c++); //sleeping simulation
            
            pthread_mutex_lock(&(IO[t]->MUTEX_io));
            shutoff = IO[t]->SHUTOFF_io;        

            if (!shutoff) {
                IO[t]->INTERRUPT_iocomplete = true;
                pthread_mutex_unlock(&(IO[t]->MUTEX_io));
                pthread_cond_wait(&(IO[t]->COND_io), &(IO[t]->MUTEX_io));                
                empty = FIFOq_is_empty(IO[t]->waitingQ, &io_error);
            }

            if (THREAD_DEBUG) printf("\t\tIO %d: IO ops finished; queue has %d PCBs left, shutoff is %s\n", t + FIRST_IO, IO[t]->waitingQ->size, shutoff ? "true" : "false");
            pthread_mutex_unlock(&(IO[t]->MUTEX_io));
            
        }
        pthread_mutex_lock(&(IO[t]->MUTEX_io));
        empty = FIFOq_is_empty(IO[t]->waitingQ, &io_error);
        if (empty && !shutoff)
            pthread_cond_wait(&(IO[t]->COND_io), &(IO[t]->MUTEX_io));
        pthread_mutex_unlock(&(IO[t]->MUTEX_io));
    } while (!shutoff);
    
    if (THREAD_DEBUG) printf("\t\tIO %d: shutting off\n", t + FIRST_IO);
    pthread_exit(NULL);
    return NULL;
}

/******************************************************************************/
/******************************** TRAPS ***************************************/
/******************************************************************************/

void trap_terminate(int* error) {
    sysStackPop(current->regs, error);
    current->state = terminated;
    current->timeTerminate = clock_;
    closeable--;
    char pcbstr[PCB_TOSTRING_LEN];
    if (OUTPUT) printf(">Terminated: %s\n", PCB_toString(current, pcbstr, error));

    FIFOq_enqueuePCB(terminateQ, current, error);
    //current = idl;
    scheduler(error);
}

void trap_iohandler(const int t, int* error) {
    sysStackPop(current->regs, error);
    current->state = waiting;
    char pcbstr[PCB_TOSTRING_LEN];
    if (OUTPUT) printf(">I/O %d: added: %s\n", t + FIRST_IO, PCB_toString(current, pcbstr, error));
    
    FIFOq_enqueuePCB(IO[t]->waitingQ, current, error);
    if (THREAD_DEBUG) printf("\t\tIO %d: gained PCB\n", t + FIRST_IO);
    scheduler(error);
}

/******************************************************************************/
/*********************** INTERRUPT SERVICE ROUTINES ***************************/
/******************************************************************************/

void interrupt(const int INTERRUPT, void* args, int* error) {
    switch (INTERRUPT) {

        case NO_INTERRUPT:
            current->state = running;
            break;

        case INTERRUPT_TIMER:
            isr_timer(error);
            break;

        case INTERRUPT_IOCOMPLETE:
            isr_iocomplete(((int) args), error);
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

void isr_iocomplete(const int t, int* error) {
    
    if (!FIFOq_is_empty(IO[t]->waitingQ, error)) {
        PCB_p pcb = FIFOq_dequeue(IO[t]->waitingQ, error);
        pcb->state = ready;
        FIFOq_enqueuePCB(readyQ[pcb->priority], pcb, error);
        char pcbstr[PCB_TOSTRING_LEN];
        if (OUTPUT) printf(">I/O %d: complete: %s\n", t + FIRST_IO, PCB_toString(pcb, pcbstr, error));

    } else if (THREAD_DEBUG) printf("ERROR! nothing to dequeue in IO %d\n", t);

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

//    PCB_p previous = current;
    PCB_p temp;
    PCB_p pcb = current;
    bool pcb_idl = current == idl;
    bool pcb_term = current->state == terminated;
    bool pcb_io = current->state == waiting;
    
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
        FIFOq_enqueuePCB(readyQ[temp->priority], temp, error);
        if (OUTPUT) {
            char pcbstr[PCB_TOSTRING_LEN];
            printf(">Enqueued to ready queue %hu: %s\n", temp->priority, PCB_toString(temp, pcbstr, error));
        }
    }
    
    if (DEBUG) printf("createQ transferred to readyQ\n");
    
    int r;
    for (r = 0; r < PRIORITIES_TOTAL; r++)
        if (!FIFOq_is_empty(readyQ[r], error))
            break;
    
    if (r == PRIORITIES_TOTAL) {// ||current->pid == readyQ[r]->head->data->pid) { //nothing in any ready queues
        if (pcb_term || pcb_io) //or in locks or conds
            current = idl;
        PCB_setState(current, running);
        sysStackPush(current->regs, error);
        return;
    }
    
    if (!(context_switch % OUTPUT_CONTEXT_SWITCH)) {
        char pcbstr[PCB_TOSTRING_LEN];
        printf(">PCB: %s\n", PCB_toString(current, pcbstr, error));
        char rdqstr[PCB_TOSTRING_LEN];
        printf(">Switching to: %s\n", PCB_toString(readyQ[r]->head->data, rdqstr, error));
    }

    
    if (!pcb_idl && !pcb_term && !pcb_io) { //add stuff about locks and conds
        current->state = ready;
        FIFOq_enqueuePCB(readyQ[current->priority], current, error);
    } else idl->state = waiting;
    dispatcher(error);

    if (!(context_switch % 4)) {
        char runstr[PCB_TOSTRING_LEN];
        printf(">Now running: %s\n", PCB_toString(current, runstr, error));
        char rdqstr[PCB_TOSTRING_LEN];
        if (!pcb_idl && !pcb_term && !pcb_io)
            if (readyQ[r]->size > 1)
                printf(">Returned to ready queue: %s\n", PCB_toString(readyQ[r]->tail->data, rdqstr, error));
            else
                printf(">No process return required.\n");
        else if (pcb_idl)
            printf(">Idle process switch run: %s\n", PCB_toString(idl, rdqstr, error));
        else if (pcb_term)
            printf(">Last process terminated: %s\n", PCB_toString(terminateQ->tail->data, rdqstr, error));
        else if (pcb_io)
            printf(">Requested I/O operation: %s\n", PCB_toString(pcb, rdqstr, error));
        int stz = FIFOQ_TOSTRING_MAX;
        char str[stz];
        printf(">%s\n", FIFOq_toString(readyQ[r], str, &stz, error));

    }

    //"housekeeping"
}

/* Dispatches a new current process by dequeuing the head of the ready queue,
 * setting its state to running and copying its CPU state onto the stack.
 */
void dispatcher(int* error) {

    int r;
    
    if (readyQ == NULL) {
        *error += FIFO_NULL_ERROR;
        printf("%s", "ERROR: readyQ is null");
    } else
        for (r = 0; r < PRIORITIES_TOTAL; r++)
            if (!FIFOq_is_empty(readyQ[r], error)) { //dequeue the head of readyQueue
                current = FIFOq_dequeue(readyQ[r], error);
                break;
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
    if (first_batch && (!PCB_CREATE_EVERY) && (rand() % PCB_CREATE_CHANCE))
        return OS_NO_ERROR;
    // random number of new processes between 0 and 5
    static int processes_created = 0;

    if (processes_created >= MAX_PROCESSES)
        return -1;
        
    int i;
    int r = rand() % (MAX_NEW_PCB + 1);
    char buffer[PCB_TOSTRING_LEN];
    
    
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
        newPcb->timeCreate = clock_;
        closeable += newPcb->regs->reg.TERMINATE > 0;
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
        if (SysPointer >= SYSSIZE - 1) {
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
    for (r = REGNUM - 1; r >= 0; r--)
        if (SysPointer <= 0) {
            *error += CPU_STACK_ERROR;
            printf("ERROR: Sysstack exceeded\n");
            toRegs->gpu[r] = STACK_ERROR_DEFAULT;
        } else {
            if (STACK_DEBUG) printf("\tpopping SysStack[%d] = %lu ", SysPointer - 1, SysStack[SysPointer - 1]);
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
    int s;
    if (DEBUG) printf("cleanup begin\n");

    //while (!pthread_mutex_trylock(&MUTEX_timer)); //wait for timer
    pthread_cond_wait(&COND_timer, &MUTEX_timer);
    SHUTOFF_timer = true;
    pthread_mutex_unlock(&MUTEX_timer);
    pthread_cond_signal(&COND_timer);
    if (THREAD_DEBUG) printf("timer shutoff signal sent\n");
    pthread_join(THREAD_timer, NULL);
    //...
    if (THREAD_DEBUG) printf("timer shutoff successful\n");
    pthread_mutex_destroy(&MUTEX_timer);
    pthread_cond_destroy(&COND_timer);

    
    for (t = 0; t < IO_NUMBER; t++) {
        char wQ[12] = "waitingQ_x";
        wQ[9] = t + '0';
        pthread_cond_signal(&(IO[t]->COND_io));

//        while (pthread_mutex_trylock(&(IO[t]->MUTEX_io)));
        pthread_mutex_lock(&(IO[t]->MUTEX_io));
        
        IO[t]->SHUTOFF_io = true;
        pthread_mutex_unlock(&(IO[t]->MUTEX_io));
        pthread_cond_signal(&(IO[t]->COND_io));
        if (THREAD_DEBUG) printf("io %d shutoff signal sent\n", t + FIRST_IO);
        pthread_join((IO[t]->THREAD_io), NULL);
        //...
        if (THREAD_DEBUG) printf("io %d shutoff successful\n", t + FIRST_IO);
        pthread_mutex_destroy(&(IO[t]->MUTEX_io));
        pthread_cond_destroy(&(IO[t]->COND_io));
        queueCleanup(IO[t]->waitingQ, wQ, error);
        free(IO[t]);
        IO[t] = NULL;
    }

    queueCleanup(terminateQ, "terminateQ", error);
    int r;
    char qustr[16];
    for (r = 0; r < PRIORITIES_TOTAL; r++) {
        sprintf(qustr, "readyQ[%d]", r);
        queueCleanup(readyQ[r], qustr, error);
    }
    queueCleanup(createQ, "createQ", error);

    int endidl = current->pid == idl->pid;
    if (idl != NULL) {
        if (EXIT_STATUS_MESSAGE) {
            char pcbstr[PCB_TOSTRING_LEN];
            printf("\n>System Idle: %s\n", PCB_toString(idl, pcbstr, error));
        }
        PCB_destruct(idl);
    }
    if (!endidl && current != NULL) {
        if (EXIT_STATUS_MESSAGE) {
            char pcbstr[PCB_TOSTRING_LEN];
            printf("\n>Running PCB: %s\n", PCB_toString(current, pcbstr, error));
        }
        PCB_destruct(current);
    } else if (EXIT_STATUS_MESSAGE) printf("\n>Idle process was final Running PCB\n");

}

/* Deallocates the queue data structures on the C simulator running this program
 * so that we don't have memory leaks and horrible garbage.
 */
void queueCleanup(FIFOq_p queue, char *qstr, int *error) {
    int stz = FIFOQ_TOSTRING_MAX;
    char str[stz];
    
    if (EXIT_STATUS_MESSAGE) {
        printf("\n>%s deallocating...\n", qstr);
        printf(">%s", FIFOq_toString(queue, str, &stz, error));
    }
    
    if (queue->size) {
        if (EXIT_STATUS_MESSAGE) {
            printf("\n>System exited with non-empty %s\n", qstr);
        }
        while (!FIFOq_is_empty(queue, error)) {
            PCB_p pcb = FIFOq_dequeue(queue, error);
            if (pcb != idl) {
                char pcbstr[PCB_TOSTRING_LEN];
                if (EXIT_STATUS_MESSAGE) printf("\t%s\n", PCB_toString(pcb, pcbstr, error));
                PCB_destruct(pcb);

            } else printf("IDL!!!!!!!!!!\n");

        }
    } else if (EXIT_STATUS_MESSAGE) printf(" empty\n");
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

/******************************************************************************/
/*************************** TESTING/DEBUG ************************************/
/******************************************************************************/

void nanosleeptest() {
    int i = 0;
    struct timespec m = {.tv_sec = 0, .tv_nsec = 5};
    while (i < 100000) {
        nanosleep(&m, NULL);
        printf("%d nanoseconds have passes", i);
        i++;
    }
}



