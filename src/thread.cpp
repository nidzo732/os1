#include "core.h"
#include "intrpt.h"
#include "core.h"
#include "syscall.h"
#include "defs.h"
#include "pcb.h"
#include "core.h"

void Thread::start()
{
	if(!myPCB) return;
	syscall(SYSCALL_START_THREAD, this->myPCB, 0);
}
void Thread::waitToComplete()
{
	if(myPCB) syscall(SYSCALL_WAIT_ON, this->myPCB, 1);
	return;
}
Thread::~Thread()
{
	if(!myPCB) return;
	waitToComplete();
	safe(delete myPCB);
	myPCB=(void*)0;
}
ID Thread::getId()
{
	if(!myPCB) return -1;
	return myPCB->id;
}
ID Thread::getRunningId()
{
	return (int)syscall(SYSCALL_GET_RUNNING_ID, (void*)0, 0);
}
Thread * Thread::getThreadById(ID id)
{
	PCB *pp=(PCB*)syscall(SYSCALL_GET_BY_ID, (void*)id, 0);
	if(pp) return pp->thread;
	else return (void*)0;
}
Thread::Thread (StackSize stackSize, Time timeSlice)
{
	safe(myPCB=new PCB(this, stackSize, timeSlice, PCB::running));
	if(!myPCB || myPCB->status==ALOFAIL)
	{
		safe(delete myPCB);
		myPCB=(void*)0;
	}
}
void dispatch()
{
	syscall(SYSCALL_DISPATCH, (void*)0, 0);
}

void Thread::signal(SignalId signal)
{
	if(!myPCB) return;
	if(signal>=16) return;
	SignalParam *p;
	safe(p=new SignalParam());
	if(!p) return;
	p->id=signal;
	p->target=this->myPCB;
	syscall(SYSCALL_SIGNAL_SIGNAL, p, 0);
	safe(delete p);
}

void Thread::registerHandler(SignalId signal, SignalHandler handler)
{
	if(!myPCB) return;
	if(signal>=16) return;
	SignalParam *p;
	safe(p=new SignalParam());
	if(!p) return;
	p->id=signal;
	p->target=this->myPCB;
	p->handler=handler;
	syscall(SYSCALL_SIGNAL_SETHANDLER, p, 0);
	safe(delete p);
}
SignalHandler Thread::getHandler(SignalId signal)
{
	if(!myPCB) return (void*)0;
	if(signal>=16) return (void*)0;
	SignalParam *p;
	safe(p=new SignalParam());
	if(!p) return (void*)0;
	p->id=signal;
	p->target=this->myPCB;
	void *handler=syscall(SYSCALL_SIGNAL_GETHANDLER, p, 0);
	safe(delete p);
	return (SignalHandler)handler;
}
void Thread::maskSignal(SignalId signal)
{
	if(!myPCB) return;
	if(signal>=16) return;
	SignalParam *p;
	safe(p=new SignalParam());
	if(!p) return;
	p->id=signal;
	p->target=this->myPCB;
	p->flags=1;
	syscall(SYSCALL_SIGNAL_MASK, p, 0);
	safe(delete p);
}
void Thread::maskSignalGlobally(SignalId signal)
{
	if(signal>=16) return;
	SignalParam *p;
	safe(p=new SignalParam());
	if(!p) return;
	p->id=signal;
	p->target=(void*)0;;
	p->flags=1;
	syscall(SYSCALL_SIGNAL_MASK, p, 0);
	safe(delete p);
}
void Thread::unmaskSignal(SignalId signal)
{
	if(!myPCB) return;
	if(signal>=16) return;
	SignalParam *p;
	safe(p=new SignalParam());
	if(!p) return;
	p->id=signal;
	p->target=this->myPCB;
	p->flags=0;
	syscall(SYSCALL_SIGNAL_MASK, p, 0);
	safe(delete p);
}
void Thread::unmaskSignalGlobally(SignalId signal)
{
	if(signal>=16) return;
	SignalParam *p;
	safe(p=new SignalParam());
	if(!p) return;
	p->id=signal;
	p->target=(void*)0;;
	p->flags=0;
	syscall(SYSCALL_SIGNAL_MASK, p, 0);
	safe(delete p);
}

void Thread::blockSignal(SignalId signal)
{
	if(!myPCB) return;
	if(signal>=16) return;
	SignalParam *p;
	safe(p=new SignalParam());
	if(!p) return;
	p->id=signal;
	p->target=myPCB;
	p->flags=1;
	syscall(SYSCALL_SIGNAL_BLOCK, p, 0);
	safe(delete p);
}
void Thread::blockSignalGlobally(SignalId signal)
{
	if(signal>=16) return;
	SignalParam *p;
	safe(p=new SignalParam());
	if(!p) return;
	p->id=signal;
	p->target=(void*)0;;
	p->flags=1;
	syscall(SYSCALL_SIGNAL_BLOCK, p, 0);
	safe(delete p);
}
void Thread::unblockSignal(SignalId signal)
{
	if(!myPCB) return;
	if(signal>=16) return;
	SignalParam *p;
	safe(p=new SignalParam());
	if(!p) return;
	p->id=signal;
	p->target=myPCB;
	p->flags=0;
	syscall(SYSCALL_SIGNAL_BLOCK, p, 0);
	safe(delete p);
}
void Thread::unblockSignalGlobally(SignalId signal)
{
	if(signal>=16) return;
	SignalParam *p;
	safe(p=new SignalParam());
	if(!p) return;
	p->id=signal;
	p->target=(void*)0;
	p->flags=0;
	syscall(SYSCALL_SIGNAL_BLOCK, p, 0);
	safe(delete p);
}
void Thread::pause()
{
	syscall(SYSCALL_SIGNAL_SLEEP, (void*)0, 0);
}
