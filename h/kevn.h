#ifndef HEADERS_KEVN_H_
#define HEADERS_KEVN_H_

#include "event.h"
class KernelEv
{
public:
	PCB *owner;
	IVTNo id;
	Event *parent;
	volatile int waiting;
	void signal();
	KernelEv::KernelEv(Event *e, IVTNo id);
	~KernelEv();
};



void setup_events();


#endif /* HEADERS_KEVN_H_ */
