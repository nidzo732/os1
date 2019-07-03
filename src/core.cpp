#include "core.h"
#include "thread.h"
#include "swrap.h"
#include "intrpt.h"
#include "syscall.h"
#include "ksem.h"
#include "kevn.h"
#include "defs.h"

class MainThread;
class NOPThread;
static MainThread *mainT;
static volatile unsigned int cSP, cSS, cBP;
static volatile unsigned long ATC = 0;
static volatile int _init_done=0;
volatile int exclusive=1;
static NOPThread *nopThread;

void timer_ticker();
void sched();
static Time currentSlice;

extern int userMain(int argc, char* argv[]);

SyscallReturn dispatch_handler(void*)
{
	sched();
	return (void*)0;
}
SyscallReturn get_running_id_handler(void*)
{
	return (void*)(PCB::running)->id;
}
SyscallReturn wait_on_handler(void *p)
{
	PCB *pcb=(PCB*)p;
	if(pcb==PCB::running) return (void*)0;
	if(pcb->status!=DONE)
	{
		pcb->waiters.push(PCB::running);
		PCB::running->preWaitStatus=PCB::running->status;
		PCB::running->status=WAITING;
		sched();
	}
	return (void*)0;
}
SyscallReturn start_thread_handler(void *t)
{
	PCB *pcb=(PCB*)t;
	if(pcb->status==PRE_RUN)
	{
		ATC++;
		pcb->status=RUNNING;
		SchedWrap::put(pcb);
	}
	return (void*)0;
}
SyscallReturn mkthread_handler(void *p)
{
	PCB *pcb=(PCB*)p;
	if(pcb->status==INVALID) pcb->init();
	return (void*)0;
}
SyscallReturn delete_thread_handler(void *p)
{
	PCB *pcb=(PCB*)p;
	safe(delete pcb);
	return (void*)0;
}
SyscallReturn get_by_id_handler(void *p)
{
	ID id=(ID)p;
	PCB *q=PCB::getById(id);
	return q;
}

SyscallReturn end_thread_handler(void*)
{
	if(PCB::running->parentID!=-1)
	{
		PCB *parent=PCB::getById(PCB::running->parentID);
		if(parent && parent->status!=DONE)
		{
			int response = parent->signal(1);
			if(response)
			{
				SchedWrap::put(parent);
			}
		}
	}
	safe(PCB::running->signal(2));
	PCB::running->handleSignals();
	(PCB::running)->status=DONE;
	sched();
}

SyscallReturn sleep_handler(void*)
{
	PCB::running->preWaitStatus=PCB::running->status;
	PCB::running->status=WAITING_SIGNAL;
	PCB::running->resignal();
	sched();
	return (void*)0;
}
SyscallReturn mask_handler(void *p)
{
	SignalParam *param = (SignalParam*)p;
	if(param->target)
	{
		param->target->mask(param->id, param->flags);
	}
	else
	{
		PCB::globalMask(param->id, param->flags);
	}
	return (void*)0;
}
SyscallReturn gethandler_handler(void *p)
{
	SignalParam *param = (SignalParam*)p;
	return param->target->handlers[param->id];
}
SyscallReturn sethandler_handler(void *p)
{
	SignalParam *param = (SignalParam*)p;
	param->target->handlers[param->id]=param->handler;
	return (void*)0;
}
SyscallReturn signal_signal_handler(void *p)
{
	SignalParam *param = (SignalParam*)p;
	int response=param->target->signal(param->id);
	if(response)
	{
		SchedWrap::put(param->target);
	}
	return (void*)0;
}
SyscallReturn block_signal_handler(void *p)
{
	SignalParam *param = (SignalParam*)p;
	if(param->target)
	{
		param->target->block(param->id, param->flags);
	}
	else
	{
		PCB::globalBlock(param->id, param->flags);
	}
	return (void*)0;
}

class MainThread: public Thread
{
public:
	int argc;
	char **argv;
	int returned;
	MainThread(int argc, char **argv)
	:Thread(8192)
	{
		this->argc=argc;
		this->argv=argv;
		this->returned=-1;
	}
	virtual void run() {
		returned=userMain(argc, argv);
	}

};
class NOPThread: public Thread
{
public:
	NOPThread()
		:Thread(512, 1)
	{
	}
	virtual void run();
};
void NOPThread::run() {
	int neki;
	while(1)
	{
		asm nop;
	}
}
int init()
{
	if(_init_done) return 0;
	asm cli;
	safe(PCB::boot=PCB::running = new PCB((Thread*)0, 10,1, (void*)0));
	((PCB*)PCB::running)->status=RUNNING;
	setup_syscall();
	setup_semaphores();
	setup_events();
	ATC=0;
	register_syscall_handler(SYSCALL_DISPATCH, dispatch_handler);
	register_syscall_handler(SYSCALL_GET_RUNNING_ID, get_running_id_handler);
	register_syscall_handler(SYSCALL_WAIT_ON, wait_on_handler);
	register_syscall_handler(SYSCALL_START_THREAD, start_thread_handler);
	register_syscall_handler(SYSCALL_MKTHREAD, mkthread_handler);
	register_syscall_handler(SYSCALL_DELETE_THREAD, delete_thread_handler);
	register_syscall_handler(SYSCALL_GET_BY_ID, get_by_id_handler);

	register_syscall_handler(SYSCALL_SIGNAL_SLEEP, sleep_handler);
	register_syscall_handler(SYSCALL_SIGNAL_MASK, mask_handler);
	register_syscall_handler(SYSCALL_SIGNAL_GETHANDLER, gethandler_handler);
	register_syscall_handler(SYSCALL_SIGNAL_SETHANDLER, sethandler_handler);
	register_syscall_handler(SYSCALL_SIGNAL_SIGNAL, signal_signal_handler);
	register_syscall_handler(SYSCALL_SIGNAL_BLOCK, block_signal_handler);

	register_syscall_handler(SYSCALL_END_THREAD, end_thread_handler);
	setup_timer(timer_ticker);
	_init_done=1;
	return 0;
}

void go(int argc, char* argv[])
{
	lock();
	safe(mainT=new MainThread(argc, argv));
	mainT->start();
	safe(nopThread=new NOPThread());
	nopThread->start();
	PCB *mt = PCB::getPCB(mainT);
	PCB::nop = PCB::getPCB(nopThread);
	while(SchedWrap::get())
	{
		SchedWrap::get();
	}
	SchedWrap::put(mt);
	PCB::mainT=mt;
	ATC=1;
	sched();
}
void timer_ticker()
{
	sem_timer_tick();
	if(!exclusive && currentSlice>0 && PCB::running->status!=HANDLING)
	{
		currentSlice--;
		sem_timer_sched();
		if(currentSlice==0) sched();
	}
}
void done()
{
	asm cli;
	for(unsigned char i=0;;i++)
	{
		restore_old(i);
		if(i==0xff) break;
	}
	PCB::running=PCB::boot;
}
void finalize_thread()
{
	ATC--;
	PCB *w=(PCB::running)->waiters.pop();
	while(w)
	{
		w->status=w->preWaitStatus;
		SchedWrap::put(w);
		w=(PCB::running)->waiters.pop();
	}
	PCB::toDelete=PCB::running;
}

int get_main_result()
{
	return mainT->returned;
}

void sched()
{
	asm{
		MOV cSP, SP
		MOV cSS, SS
		MOV cBP, BP
	}
	PCB::running->SP=cSP;
	PCB::running->SS=cSS;
	PCB::running->BP=cBP;
	if(PCB::running->status==RUNNING && PCB::running!=PCB::nop && PCB::running!=PCB::boot)
	{
		SchedWrap::put((PCB*)PCB::running);
	}
	if(PCB::running->status==DONE)
	{
		finalize_thread();
	}
	PCB::running=SchedWrap::get();
	while(PCB::running && PCB::running->status!=RUNNING && PCB::running->status!=HANDLING)
	{
		PCB::running=SchedWrap::get();
	}
	if(!PCB::running)
	{
		if(!ATC)
		{
			done();
		}
		else
		{
			PCB::running=PCB::nop;
		}
	}
	currentSlice=PCB::running->slice;
	cSP=PCB::running->SP;
	cSS=PCB::running->SS;
	cBP=PCB::running->BP;
	asm{
		MOV SP, cSP;
		MOV SS, cSS;
		MOV BP, cBP;
	}
	if(PCB::toDelete)
	{
		safe(delete[] PCB::toDelete->stack);
		PCB::toDelete->stack=(void*)0;
		PCB::toDelete=(void*)0;
	}
	if(PCB::running->status!=HANDLING) PCB::running->handleSignals();
	exclusive=0;
	return;
}
