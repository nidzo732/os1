#ifndef HEADERS_SIGQUEUE_H_
#define HEADERS_SIGQUEUE_H_

class SigQueue
{
protected:
	struct QElem
	{
		QElem *next;
		int d;
	};
	QElem *first, *last;
	int size;
public:
	SigQueue();
	void push(int signal);
	int pop();
	int getSize();
	~SigQueue();
};


#endif
