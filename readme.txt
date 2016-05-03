things to do:

Bun
	GitHub and trap functions, isr
	timer thread

Chris
	integrate anything missed in Chris's stuff
	IO thread, other threads or functions?

Mark
	once everyone submits to GitHub, mark will debug

Paul
	read up on everything and understand it well enough to do what needed


C pocket reference cuz there's a cow on the front

4 threads running:
	CPU
	Timer
	I/O 1
	I/O 2

Changes:
	union PCB or CPU registers...........
	PCB expanded struct (bunch of u-longs and 2 ulong arrays)
		MAX_PC		rollover number
		timeCreate	ms CPU time at creation
		timeTerminate	ms CPU time at termination
		TERMINATE	0 for infinite, >0 for number of rollovers
		terminateCount	how many times MAX_PC has been hit
		IO_1_TRAPS[4]	PC number at trap 1 execution
		IO_2_TRAPS[4]	PC number at trap 2 execution

	New OS.c class fields (all mutex protected)
		a bunch of new mutexes and conditions and stuff
		fifoQ terminateQ
		boolean		INTERRUPT_TIMER
		array of boolean or something like a boolean
			boolean		INTERRUPT_IO_1_COMPLETE
			boolean		INTERRUPT_IO_2_COMPLETE
		array of fifoQ (better: array of structs with a bool and fifoq)
			fifoQ waitingQ_IO_1
			fifoQ waitingQ_IO_2

	CPU redesigned to simulate 1 PC increment per loop
		before CPU starts, start IO and timer threads
		needs to have copies of PCB's non-constant factors"
	CPU loop (Main_OS_Loop or something): - Mark
		1 PC++
		2 check if PC == MAX_PC
			if true PC = 0, terminateCount+
			check if terminateCount = TERMINATE
				if true, call trap_terminate
		3 check for INTERRUPT_TIMER
			if true call isrTimer(... see .h) alrady exists
		4 check for IO_COMPLETE 1 and 2
			if true call isrIOComplete
		5 switch: check's PC against both IO[4] arrays
			true call trap_IO_handler(TRAP_SRN) TRAP_SRN is 1 or 2

	sysStackSaveState(PCB or CPU) - Mark after Chris struct
		puts all (PCB or CPU) contents onto stack	

	sysStackRestoreState(PCB or CPU) - Mark after Chris struct
		restores to (PCB or CPU) contents of the stack

	trap_terminate - Bun
		pops CPU info from stack and copies to current PCB
		changesa current running process state to terminated
		puts that process in a terminate readyQ
		calls scheduler->dispatch new running process	

	trap_IO_handler(TRAP_SRN)   - Bun
		SRN = service routine number
		change running process to waiting, put in IO_x queue
		signal to IO_SRN_TRAP thread to start internal timer
		call scheduler->dispatch new running process

	isrIOComplete(io number)   - Bun
		takes the head of the given IO queue and places it in the back
			of the readyQ
		sets IO_x_COMPLETE to false
		signals IO_x that it can resume IO-ing

	isrTimer(see .h) - Mark check if still good
		changes out current running process and scheduler->dispatcher

	timer(???) thread - Chris
		puts itself to sleep or has a long loop
		on wakeup/loop end, timer interrupt
		while (true)
			for (timer begin; timer end; timer++) //or nanosleep()
				//nothing
			signal CPU that timer is interrupting (calling isrTimer?)

	io(???) thread x2 - Bun
		puts istelf to sleep or has a long loop

		while (items to process IO)
			for (io timer being; io timer end, io timer++) //or ns()
				//nothgin
			signals interrupt to CPU