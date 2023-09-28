all: scheduler
scheduler: main.o system.o
	gcc -o scheduler main.o system.o

main.o: main.c system.c
	gcc -o main.o -c main.c

system.o: system.c system.h scheduling_queue.h task.h
	gcc -o system.o -c system.c

clean: 
	rm -rf rm scheduler *.o
	
