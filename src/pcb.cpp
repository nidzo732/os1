#include <dos.h>
#include "swrap.h"
#include "defs.h"
#include "pcb.h"
#include "syscall.h"
#include "core.h"
static unsigned int dsFT;

static unsigned char globalSignalMask[16]={0};

static void default_0_handler()
{
	syscall(SYSCALL_END_THREAD, (void*)0);
}
PCB::PCB(Thread *t, StackSize sSize, Time tslice, PCB *parent)
{
	thread=t;
	slice=tslice;
	handlers[0]=default_0_handler;
	signalMask[0]=0;
	for(int i=1;i<16;i++)
	{
		signalMask[i]=0;
		handlers[i]=(void*)0;
	}
	if(sSize<512) sSize=512;
	safe(stack=new unsigned int[sSize/2]);
	if(!stack)
	{
		status=ALOFAIL;
		return;
	}
	if(parent)
	{
		this->parentID=parent->id;
		for(int i=1;i<16;i++)
		{
			signalMask[i]=parent->signalMask[i];
			handlers[i]=parent->handlers[i];
		}
	}
	else this->parentID=-1;
	int csp=sSize/2;
	unsigned int ip=FP_OFF(threadrunner);
	unsigned int cs=FP_SEG(threadrunner);
	unsigned int bp=FP_OFF(&(stack[csp-1]));
	stack[--csp]=cs;
	stack[--csp]=ip;
	stack[--csp]=bp;
	asm MOV dsFT, DS;
	stack[--csp]=dsFT;
	SP=FP_OFF(&(stack[csp]));
	SS=FP_SEG(&(stack[csp]));
	BP=bp;
	status=INVALID;
	preWaitStatus=INVALID;
	id=-1;
	holder=(void*)0;   //null
	syscallResponse=-1;
	stackSize=sSize/2;
	init();
}
void PCB::init()
{
	status=PRE_RUN;
	id=gId++;
	safe(holder=new LElem);
	if(!holder)
	{
		status=ALOFAIL;
		return;
	}
	holder->d=this;
	if(first)
	{
		holder->prev=last;
		last=last->next=holder;
		last->next=(void*)0;
	}
	else
	{
		first=last=holder;
		holder->next=(void*)0;
		holder->prev=(void*)0;
	}
}
void PCB::threadrunner()
{
	asm sti;
	PCB::running->thread->run();
	syscall(SYSCALL_END_THREAD, (void*)0);
}
PCB* PCB::getById(ID id)
{
	LElem *c=first;
	while(c)
	{
		if(c->d->id==id) return c->d;
		c=c->next;
	}
	return (void*)0;
}
PCB::~PCB()
{
	if(holder->next)
	{
		holder->next->prev=holder->prev;
	}
	if(holder->prev)
	{
		holder->prev->next=holder->next;
	}
	if(holder==first)
	{
		first=first->next;
	}
	if(holder==last)
	{
		last=last->prev;
	}
	safe(delete holder);
	if(stack) safe(delete[] stack);
	stack=(void*)0;
	if(PCB::toDelete==this) PCB::toDelete=(void*)0;
}

PCB* PCB::getPCB(Thread *t)
{
	return t->myPCB;
}
void PCB::handleSignals()
{
	int size=signals.getSize();
	_status startingStatus=this->status;
	_status pwStatus=this->preWaitStatus;
	status=HANDLING;
	for(int i=0;i<size;i++)
	{
		int signal=signals.pop();
		if(signal<16)
		{
			if(globalSignalMask[signal] || signalMask[signal])
			{
				signals.push(signal);
			}
			else if(handlers[signal])
			{
				int oldex=exclusive;
				exclusive=1;
				asm sti;
				handlers[signal]();
				asm cli;
				exclusive=oldex;
			}
		}
	}
	this->status=startingStatus;
	this->preWaitStatus=pwStatus;
}
void PCB::globalBlock(SignalId id, unsigned char flags)
{
	if(flags)
	{
		globalSignalMask[id]|=1;
	}
	else
	{
		globalSignalMask[id]&=(~1);
		LElem *c=PCB::first;
		while(c)
		{
			c->d->resignal();
			c=c->next;
		}
	}
}
void PCB::globalMask(SignalId id, unsigned char flags)
{
	if(flags)
	{
		globalSignalMask[id]|=2;
	}
	else
	{
		globalSignalMask[id]&=(~2);
	}
}
void PCB::block(SignalId id, unsigned char flags)
{
	if(flags)
	{
		signalMask[id]|=1;
	}
	else
	{
		signalMask[id]&=(~1);
		resignal();
	}
}
void PCB::resignal()
{
	if(status==WAITING_SIGNAL)
	{
		int size=signals.getSize();
		int x=0;
		for(int i=0;i<size;i++)
		{
			x|=signal(signals.pop());
		}
		if(x)SchedWrap::put(this);
	}
}
void PCB::mask(SignalId id, unsigned char flags)
{
	if(flags)
	{
		signalMask[id]|=2;
	}
	else
	{
		signalMask[id]&=(~2);
	}
}
int PCB::signal(SignalId id)
{
	if(signalMask[id]&2) return 0;
	if(globalSignalMask[id]&2) return 0;
	signals.push(id);
	if(signalMask[id]&1) return 0;
	if(globalSignalMask[id]&1) return 0;
	if(status==WAITING_SIGNAL)
	{
		status=preWaitStatus;
		return 1;
	}
	return 0;
}
void PCB::setHandler(SignalId id, SignalHandler handler)
{
	handlers[id]=handler;
}
ID PCB::gId=0;
PCB::LElem* PCB::first=(LElem*)0;
PCB::LElem* PCB::last=(LElem*)0;
PCB* PCB::running=(PCB*)0;
PCB* PCB::boot=(PCB*)0;
PCB* PCB::nop=(PCB*)0;
PCB* PCB::toDelete=(PCB*)0;
PCB* PCB::mainT=(PCB*)0;
