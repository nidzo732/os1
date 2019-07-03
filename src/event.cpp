#include "event.h"
#include "intrpt.h"
#include "core.h"
#include "pcbqueue.h"
#include "defs.h"
#include "syscall.h"
#include "kevn.h"


Event::Event(IVTNo ivtNo)
{
	safe(myImpl=new KernelEv(this, ivtNo));
}
Event::~Event ()
{
	if(myImpl) safe(delete myImpl);
	myImpl=(void*)0;
}
void Event::wait()
{
	if(myImpl) syscall(SYSCALL_EVENT_WAIT, myImpl, 1);
}

void Event::signal()
{
	if(myImpl) safe(myImpl->signal());
}
int reg_evn_hand(int id, intH handler)
{
	if(id!=8)register_handler(id, handler);
	return 42;
}
