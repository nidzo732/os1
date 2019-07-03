#include <dos.h>
#include "intrpt.h"
#include "core.h"
#include "core.h"
#include "event.h"

static char old_value_exists[256]={0};
static intH old_values[256];
void (*timer_handler)();
extern void tick();

void set_v(unsigned char id,intH handler)
{
	unsigned segM, off, pos, pos2;
	segM=FP_SEG(handler);
	off=FP_OFF(handler);
	pos=4*id;
	pos2=pos+2;
	asm{
		PUSH BX;
		PUSH DS;
		PUSH AX;

		MOV AX, 0h;
		MOV DS, AX;

		MOV BX, pos;
		MOV AX, off;
		MOV [BX], AX;

		MOV BX, pos2;
		MOV AX, segM;
		MOV [BX], AX;

		POP AX;
		POP DS;
		POP BX;
	}
}
intH get_v(unsigned char id)
{
	unsigned segM, off, pos, pos2;
	pos=4*id;
	pos2=pos+2;
	asm{
		PUSH BX;
		PUSH DS;
		PUSH AX;

		MOV AX, 0h;
		MOV DS, AX;

		MOV BX, pos;
		MOV AX, [BX];
		MOV off, AX;

		MOV BX, pos2;
		MOV AX, [BX];
		MOV segM, AX;

		POP AX;
		POP DS;
		POP BX;
	}
	return (intH)MK_FP(segM, off);
}
void register_handler(unsigned char id, intH handler)
{
	asm pushf;
	asm cli;
	old_values[id]=get_v(id);
	old_value_exists[id]=1;
	set_v(id, handler);
	asm popf;
}
void restore_old(unsigned char id)
{
	if(old_value_exists[id]) set_v(id, old_values[id]);
}
char already_set(unsigned char id)
{
	return old_value_exists[id];
}
void call_old(unsigned char id)
{
	old_values[id]();
}
void interrupt timer(...)
{
	call_old(8);
	tick();
	IVTEntry::handle(8, 0);
	if(timer_handler) timer_handler();
}
void setup_timer(void (*target)())
{
	unsigned int z=0xffff;
	asm{
		MOV CX, z
		MOV AL, CL
		OUT 40h, AL
		MOV AL, CH
		OUT 40h, AL
	};
	register_handler(8, timer);
	timer_handler=target;
}


