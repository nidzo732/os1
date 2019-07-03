#ifndef HEADERS_EVENT_H_
#define HEADERS_EVENT_H_
#include "intrpt.h"

typedef unsigned char IVTNo;
class KernelEv;
class Event {
	public:
		Event (IVTNo ivtNo);
		~Event ();
		void wait ();
	protected:
		friend class KernelEv;
		void signal(); // can call KernelEv
	private:
		KernelEv* myImpl;
};

class IVTEntry
{
public:
	static void handle(int id, int useold);
};
int reg_evn_hand(int id, intH hand);
#define PREPAREENTRY(id,useold)\
	class Entry_##id: public IVTEntry{\
		private:\
			int _handl_ev_##id;\
		public:\
			static interrupt void execute(...){\
				handle(id, useold);\
			}\
	};\
	int _handl_ev_##id=reg_evn_hand(id, Entry_##id::execute);


#endif /* HEADERS_EVENT_H_ */
