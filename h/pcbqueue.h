#ifndef HEADERS_PCBQUEUE_H_
#define HEADERS_PCBQUEUE_H_

class PCB;

class PCBQueue
{
protected:
	struct QElem
	{
		QElem *next;
		PCB *d;
	};
	QElem *first, *last;
public:
	PCBQueue();
	void push(PCB *pcb);
	PCB *pop();
	~PCBQueue();
};


#endif
