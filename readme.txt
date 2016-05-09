things to do:

C pocket reference cuz there's a cow on the front

4 threads running:
	CPU
	Timer
	I/O 1
	I/O 2

todo:
    comment everything
    bugfix: chance of lockup on IO on most recent update: about 10%?
	right after I/O adds a process, system locks up (no timer, so all locked)
    add timeCreate, timeTerminate to pcb printout
    create new thread for creating PCBs called void* user(void*)
	need: mutexes to be shared with scheduler
    simulate timer interrupt in isr, traps that aren't timer related
	simulate interrupt hierarchy?