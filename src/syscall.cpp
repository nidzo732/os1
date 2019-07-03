#include "intrpt.h"
#include "syscall.h"
#include "core.h"
#include "defs.h"

static volatile SyscallHandler handlerTable[SYSCALL_TABLE_SIZE];
static volatile SyscallCode currentCode;
static volatile SyscallParam currentParams;
static volatile SyscallReturn currentReturn;
static int _init_done=0;
void interrupt syscall_handler(...)
{
	currentReturn=handlerTable[currentCode](currentParams);
}
void setup_syscall()
{
	_init_done=1;
}
SyscallReturn syscall(SyscallCode code, SyscallParam param, int intlock)
{
	if(!_init_done) init();
	if(intlock)
	{
		asm pushf;
		asm cli;
		int pex;
		safe(pex=exclusive;
			 exclusive=1;);
		currentCode=code;
		currentParams=param;
		syscall_handler();
		SyscallReturn q=currentReturn;
		exclusive=pex;
		asm popf;
		return q;
	}
	else
	{
		int pex;
		safe(pex=exclusive;
			 exclusive=1;);
		currentCode=code;
		currentParams=param;
		syscall_handler();
		SyscallReturn q=currentReturn;
		exclusive=pex;
		return q;
	}
}
void register_syscall_handler(SyscallCode code, SyscallHandler handler)
{
	handlerTable[code]=handler;
}



