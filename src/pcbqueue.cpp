#include "pcbqueue.h"
#include "pcb.h"
#include "defs.h"

PCBQueue::PCBQueue()
{
	first=last=(QElem*)0;
}
void PCBQueue::push(PCB *pcb)
{
	QElem *x;
	safe(x=new QElem);
	if(!x) return;
	if(first)
	{
		last=last->next=x;

	}
	else
	{
		last=first=x;
	}
	last->next=(QElem*)0;
	last->d=pcb;
}

PCB* PCBQueue::pop()
{
	if(first)
	{
		QElem *z=first;
		first=first->next;
		PCB *p=z->d;
		delete z;
		return p;
	}
	else
	{
		return (PCB*)0;
	}
}

PCBQueue::~PCBQueue()
{
	while(first)
	{
		QElem *z=first->next;
		delete first;
		first =z;
	}
}
