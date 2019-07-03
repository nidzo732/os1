#include "defs.h"
#include "swrap.h"
class Scheduler{
public:
	static void put (PCB *);
	static PCB* get ();
};
void SchedWrap::put(PCB *p)
{
	asm pushf;
	asm cli;
	Scheduler::put(p);
	asm popf;
}
PCB* SchedWrap::get()
{
	PCB *p;
	asm pushf;
	asm cli;
	p=Scheduler::get();
	asm popf;
	return p;
}
