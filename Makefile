problem2: OS.o FIFOq.o PCB.o
	gcc -o problem2 OS.o FIFOq.o PCB.o

OS.o:
	gcc -c OS.c

FIFOq.o:
	gcc -c FIFOq.c

PCB.o:
	gcc -c PCB.c

