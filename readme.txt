things to do:

Mark
	OS.c scheduling algorithm; priorityQ; createQ thread

Bruno
	PCB.c; PCB.h; add timeCreate/Terminate to PCB printout; IO bugfix

Bun
	communication of PCB and fthread, scheduler; new print statements

Chris
	fthread.c; fthread.h

C pocket reference cuz there's a cow on the front

5+ threads running:
	CPU
	Timer
	I/O 1
	I/O 2
	CreatePCBs thread

todo:
    comment everything
    bugfix: chance of lockup on IO on most recent update: about 10%?
	right after I/O adds a process, system locks up (no timer, so all locked)
    add timeCreate, timeTerminate to pcb printout
    simulate timer interrupt in isr, traps that aren't timer related
	simulate interrupt hierarchy?

-----------------------------------------------
OS.c
-----------------------------------------------
create new thread for creating PCBs called void* user(void*)
	need: mutexes to be shared with scheduler

create new thread for terminating as well?

new checks for the mutex/cond arrays

bunch of new static coutners for PCB types and totals
	shceduler add to readyQ ++
	terminate deletion --

parallel global array containing pcb pro-con structs and integer resource


special deadlock monitor
every time it runs, checkings all of the mutex queues
if a pair pcb has one pcb locking mutex 1 and the other locking mutex 2
	then deadlock was foundederest!



scheduling? how do we simulate?
	if pcb checks lock and its partner has the lock, it goes into the waitingq in mutex
		it has to call some kind of interrupt to scheduler
		and the pcb itself does NOT go back into readyQ but into mutex queue


SUPER SCHEDULER CHANGES
	four priorities
		level 0,  5% only CPU types, always run that first: round-robin
		level 1, 80% all types, always run that first: round-robin and starvation watch
		level 2, 10% all types, always run that first: round-robin and starvation watch
		level 3,  5% all types, always run that first: round-robin and starvation watch

	the scheduler every 10-20 schedulings will go through all processes in all levels and
		check their internal last clock against current clock--any pcbs that have a
		significant amount of difference are moved the the back of the next highest
		readyQ

	when the scheduler re-enqueues a process, it always puts it in its original priority level

	if every a priority 0 process is ready to re-enter the queue, a special interrupt needs
		to happen that immediately calls the scheduler to start running the priority 0
		process	

-----------------------------------------------
PCB.c
-----------------------------------------------

PCB types
IO Processes		50 max
CPU Processes		25 max
Producer Processes	10 max	
Consumer Processes	10 max
Mutual 2R A Processes	? max
Mutual 2R B Processes	? max

struct pair {
	pcbA
	pcbB
	mutex
	cond
	special resource to be shared and cared for :)
}

PCB now has 4 priority levels
0 -  5%
1 - 80%
2 - 10%
3 -  5%

PCB new data fields
last	clock at last time pcb was running
type	which type
pair	the pid of the paired PCB
or index in global array of struct pcb pairs

regfile new data fields
thread arrays of pc values


prod
pc 400 lock
pc 450 wait if con hasn't read yet
pc 510 change value
pc 520 set flag to unread
pc 550 signal
pc 600 unlock

con
pc 200 lock
pc 250 wait if pro hasn't written yet
pc 310 read value
pc 320 set flag to read
pc 350 signal
pc 400 unlock

mutual 2r (non-deadlock A and B and deadlock A)

pc 600 lock mutex for resource 1
pc 650 lock mutex for resource 2
pc 700 "modify" resource 1
pc 750 "modify" resource 2
pc 800 unlock mutex for resource 2
pc 850 unlock mutex for resource 1

mutual 2r (deadlock for B only)

pc 600 lock mutex for resource 2
pc 650 lock mutex for resource 1
pc 700 "modify" resource 1
pc 750 "modify" resource 2
pc 800 unlock mutex for resource 1
pc 850 unlock mutex for resource 2




-----------------------------------------------
fthread.c
-----------------------------------------------

Mutex malloc
PCB pointer to current owner of lock
fifoQ of processes waiting for the lock


Condition malloc
fifoQ of processes waiting for signal
parallel fifoQ of the mutex for each process


lock()
unlock()
	mutex then must put head of queue into readyQ
wait()
signal()
	cond puts head of queue into its mutex
trylock()