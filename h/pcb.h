#ifndef HEADERS_PCB_H_
#define HEADERS_PCB_H_
#include "pcbqueue.h"
#include "thread.h"
#include "sigqueue.h"

enum _status { INVALID, PRE_RUN, RUNNING, HANDLING, WAITING, WAITING_SIGNAL, DONE, ALOFAIL };

class PCB
{
	struct LElem
	{
		PCB *d;
		LElem *next;
		LElem *prev;
	};
	static LElem *first, *last;
public:
	static ID gId;

	SignalHandler handlers[16];
	unsigned char signalMask[16];
	ID id;
	int syscallResponse;
	unsigned int SP;
	unsigned int SS;
	unsigned int BP;
	_status status, preWaitStatus;
	unsigned int *stack;
	LElem *holder;
	Time slice;
	SigQueue signals;
	Thread *thread;
	StackSize stackSize;
	ID parentID;
	static PCB *running;
	static PCB *nop;
	static PCB *boot;
	static PCB *toDelete;
	static PCB *mainT;
	PCBQueue waiters;
	PCB(Thread *t, StackSize sSize, Time timeslice, PCB *parent);
	void init();
	~PCB();
	static void threadrunner();
	static PCB* getById(ID id);
	void handleSignals();
	void block(SignalId id, unsigned char flags);
	void mask(SignalId id, unsigned char flags);
	static void globalBlock(SignalId id, unsigned char flags);
	static void globalMask(SignalId id, unsigned char flags);
	int signal(SignalId id);
	void setHandler(SignalId id, SignalHandler handler);
	void resignal();
	static PCB* getPCB(Thread *t);
};


#endif /* HEADERS_PCB_H_ */
