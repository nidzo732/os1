#ifndef HEADERS_THREADS_H_
#define HEADERS_THREADS_H_
#include "pcb.h"

#define lock() asm cli
#define unlock() asm sti
volatile extern int exclusive;

struct SignalParam
{
	unsigned char id;
	unsigned char flags;
	PCB *target;
	void (*handler)();
};

int init();
void go(int argc, char* argv[]);
void sched();
void start_thread(PCB *thread);
int get_main_result();


#endif /* HEADERS_THREADS_H_ */
