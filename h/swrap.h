#ifndef BCC_SCHEDWRAP_H_
#define BCC_SCHEDWRAP_H_

class PCB;

class SchedWrap
{
private:
public:
	static void put(PCB *p);
	static PCB *get();
};


#endif
