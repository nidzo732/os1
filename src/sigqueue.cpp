#include "sigqueue.h"
#include "defs.h"

SigQueue::SigQueue()
{
	first=last=(QElem*)0;
	size=0;
}
void SigQueue::push(int sig)
{
	QElem *x;
	safe(x=new QElem);
	if(!x) return;
	if(first)
	{
		last=last->next=x;
	}
	else
	{
		last=first=x;
	}
	last->next=(QElem*)0;
	last->d=sig;
	size++;
}

int SigQueue::pop()
{
	if(first)
	{
		QElem *z=first;
		first=first->next;
		int p=z->d;
		safe(delete z);
		size--;
		return p;
	}
	else
	{
		return -1;
	}
}
int SigQueue::getSize()
{
	return size;
}

SigQueue::~SigQueue()
{
	while(first)
	{
		QElem *z=first->next;
		safe(delete first);
		first =z;
	}
}
