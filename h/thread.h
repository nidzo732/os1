#ifndef _thread_h_
#define _thread_h_

typedef unsigned long StackSize;
const StackSize defaultStackSize = 4096;
typedef unsigned int Time; // time, x 55ms
const Time defaultTimeSlice = 2; // default = 2*55ms
typedef int ID;
class PCB; // Kernel's implementation of a user's thread

typedef void (*SignalHandler)();
typedef unsigned SignalId;

class Thread {
	public:
		void start();
		void waitToComplete();
		virtual ~Thread();
		ID getId();
		static ID getRunningId();
		static Thread * getThreadById(ID id);

		//SIGNAL
		void signal(SignalId signal);

		//REG/GET
		void registerHandler(SignalId signal, SignalHandler handler);
		SignalHandler getHandler(SignalId signal);

		//MASK
		void maskSignal(SignalId signal);
		static void maskSignalGlobally(SignalId signal);
		void unmaskSignal(SignalId signal);
		static void unmaskSignalGlobally(SignalId signal);
		void blockSignal(SignalId signal);
		static void blockSignalGlobally(SignalId signal);
		void unblockSignal(SignalId signal);
		static void unblockSignalGlobally(SignalId signal);

		//PAUSE
		static void pause();

	protected:
		friend class PCB;
		Thread (StackSize stackSize = defaultStackSize, Time timeSlice = defaultTimeSlice);
		virtual void run() {}
	private:
		PCB* myPCB;
};
void dispatch ();

#endif
