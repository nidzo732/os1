#include "semaphor.h"
#include "core.h"
#include "pcb.h"
#include "ksem.h"
#include "syscall.h"
#include "defs.h"
#include "swrap.h"

SyscallReturn sema_down_handler(void*);
SyscallReturn sema_up_handler(void*);
volatile static unsigned long globalTime;

class SQueue
{
private:
	struct QElem
	{
		QElem *prev1, *prev2, *next1, *next2;
		SQueue *owner;
		KernelSem *sOwner;
		PCB *p;
		unsigned long t;
		QElem(PCB *pp, unsigned long tt, SQueue *ow, KernelSem *sow)
			:p(pp), t(tt), owner(ow), prev1((void*)0), next1((void*)0), prev2((void*)0), next2((void*)0), sOwner(sow){}
	};
	QElem *first, *last;
	static QElem *tFirst, *tLast;
public:
	SQueue();
	~SQueue();
	void push(PCB *p, unsigned long t, KernelSem *ow);
	PCB* pop();
	static PCB* tPop(unsigned long t);
};
SQueue::SQueue()
	:first((void*)0), last((void*)0){}
SQueue::~SQueue()
{
	while(pop());
}
class KernelSem
{
public:
	int value;
	SQueue waiters;
};
void SQueue::push(PCB *p, unsigned long t, KernelSem *ow)
{
	QElem *x;
	safe(x=new QElem(p, t+globalTime, this,ow));
	if(!x) return;
	if(first)
	{
		last->next1=x;
		x->prev1=last;
		last=x;
	}
	else
	{
		first=last=x;
	}
	if(t!=0)
	{
		if(tFirst)
		{
			QElem *c=tFirst;
			if(c->t>t)
			{
				x->next2=c;
				c->prev2=x;
				tFirst=x;
			}
			else
			{
				while(c->next2 && c->next2->t<t) c=c->next2;
				x->prev2=c;
				x->next2=c->next2;
				if(c->next2) c->next2->prev2=x;
				else tLast=x;
				c->next2=x;
			}
		}
		else
		{
			tFirst=tLast=x;
		}
	}
}
PCB* SQueue::pop()
{
	if(first)
	{
		QElem *z=first;
		first=first->next1;
		if(first)first->prev1=(void*)0;
		PCB *tr=z->p;
		if(z->t)
		{
			if(z->next2)
			{
				z->next2->prev2=z->prev2;
			}
			if(z->prev2)
			{
				z->prev2->next2=z->next2;
			}
			if(z==tFirst) tFirst=z->next2;
			if(z==tLast) tLast=z->prev2;
		}
		safe(delete z);
		return tr;
	}
	else
	{
		return (void*)0;
	}
}
PCB* SQueue::tPop(unsigned long t)
{
	if(tFirst && tFirst->t<=t)
	{
		QElem *z=tFirst;
		tFirst=tFirst->next2;
		if(tFirst) tFirst->prev2=(void*)0;
		PCB *tr=z->p;

		if(z->next1)
		{
			z->next1->prev1=z->prev1;
		}
		if(z->prev1)
		{
			z->prev1->next1=z->next1;
		}
		if(z==z->owner->first) z->owner->first=z->next1;
		if(z==z->owner->last) z->owner->last=z->prev1;
		z->sOwner->value++;
		safe(delete z);
		return tr;
	}
	else
	{
		return (void*)0;
	}
}

SQueue::QElem* SQueue::tFirst=(SQueue::QElem*)0;
SQueue::QElem* SQueue::tLast=(SQueue::QElem*)0;




struct WaitParam
{
	Time timeout;
	KernelSem *sem;
};
Semaphore::Semaphore (int init)
{
	safe(myImpl=new KernelSem());
	if(!myImpl) return;
	myImpl->value=init;
}
Semaphore::~Semaphore ()
{
	if(!myImpl) return;
	safe(delete myImpl);
	myImpl=(void*)0;
}
int Semaphore::wait (Time maxTimeToWait)
{
	WaitParam *p;
	if(!myImpl) return -1;
	safe(p=new WaitParam);
	if(!p) return -1;
	p->sem=myImpl;
	p->timeout=maxTimeToWait;
	return (int)syscall(SYSCALL_SEMA_DOWN, p,0);
}
void Semaphore::signal()
{
	if(!myImpl) return;
	syscall(SYSCALL_SEMA_UP, myImpl, 0);
}
int Semaphore::val () const
{
	if(!myImpl) return 0;
	return myImpl->value;
}
void setup_semaphores()
{
	globalTime=0;
	register_syscall_handler(SYSCALL_SEMA_UP, sema_up_handler);
	register_syscall_handler(SYSCALL_SEMA_DOWN, sema_down_handler);
}
SyscallReturn sema_down_handler(void* p)
{
	WaitParam *w=(WaitParam*)p;
	if(w->sem->value>0)
	{
		w->sem->value--;
		safe(delete p);
		return (void*)1;
	}
	else
	{
		w->sem->value--;
		PCB::running->preWaitStatus=PCB::running->status;
		PCB::running->status=WAITING;
		safe(w->sem->waiters.push((PCB*)PCB::running, w->timeout, w->sem));
		safe(delete p);
		sched();
		return (void*)PCB::running->syscallResponse;
	};
}
SyscallReturn sema_up_handler(void* p)
{
	KernelSem *s=(KernelSem*)p;
	PCB *toFree;
	s->value++;
	safe(toFree=s->waiters.pop());
	if(toFree)
	{
		toFree->status=toFree->preWaitStatus;
		toFree->syscallResponse=1;
		SchedWrap::put(toFree);
	}
	return (void*)0;
}
void sem_timer_tick()
{
	globalTime+=1;
}
void sem_timer_sched()
{
	PCB *p=SQueue::tPop(globalTime);
	while(p)
	{
		p->status=p->preWaitStatus;
		p->syscallResponse=0;
		SchedWrap::put(p);
		p=SQueue::tPop(globalTime);
	}
}
