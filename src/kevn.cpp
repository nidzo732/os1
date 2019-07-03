#include "core.h"
#include "kevn.h"
#include "event.h"
#include "intrpt.h"
#include "syscall.h"
#include "swrap.h"

static volatile KernelEv* ev_handlers[256]={0};
static char volatile registered[256]={0};

SyscallReturn wait_handler(void *p);

void tie(KernelEv *ev)
{
	registered[ev->id]=1;
	ev_handlers[ev->id]=ev;
}

void untie(KernelEv *ev)
{
	registered[ev->id]=0;
}

KernelEv::KernelEv(Event *e, IVTNo id)
{
	owner=PCB::running;
	this->id=id;
	parent=e;
	waiting=0;
}

void KernelEv::signal()
{
	if(waiting)
	{
		untie(this);
		owner->status=owner->preWaitStatus;
		waiting=0;
		SchedWrap::put(owner);
		if(!exclusive) sched();
	}
}

void IVTEntry::handle(int id, int useold)
{
	if(useold)
	{
		call_old(id);
	}
	if(registered[id])
	{
		((KernelEv*)ev_handlers[id])->signal();
	}
	else if(id==9 && !useold)
	{
		call_old(id);
	}
}

void setup_events()
{
	register_syscall_handler(SYSCALL_EVENT_WAIT, wait_handler);
}

SyscallReturn wait_handler(void *p)
{
	KernelEv *e=(KernelEv*)p;
	if(PCB::running==e->owner || (PCB::running==PCB::mainT && e->owner==(void*)0))
	{
		if(e->owner==(void*)0) e->owner=PCB::running;
		tie(e);
		PCB::running->preWaitStatus=PCB::running->status;
		PCB::running->status=WAITING;
		e->waiting=1;
		sched();
	}
	return (void*)0;
}

KernelEv::~KernelEv()
{
	untie(this);
}
