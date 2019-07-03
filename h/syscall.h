#ifndef HEADERS_SYSCALL_H_
#define HEADERS_SYSCALL_H_

#define SYSCALL_INTERRUPT 0x61
#define SYSCALL_TABLE_SIZE 256

typedef unsigned char SyscallCode;
typedef void* SyscallParam;
typedef void* SyscallReturn;
typedef SyscallReturn (*SyscallHandler)(void*);

enum SyscallCodes {SYSCALL_DISPATCH,
				   SYSCALL_MKTHREAD,
				   SYSCALL_START_THREAD,
				   SYSCALL_GET_RUNNING_ID,
				   SYSCALL_WAIT_ON,
				   SYSCALL_DELETE_THREAD,
				   SYSCALL_GET_BY_ID,
				   SYSCALL_END_THREAD,

				   SYSCALL_SEMA_UP,
				   SYSCALL_SEMA_DOWN,

				   SYSCALL_EVENT_WAIT,

				   SYSCALL_SIGNAL_SIGNAL,
				   SYSCALL_SIGNAL_SLEEP,
				   SYSCALL_SIGNAL_MASK,
				   SYSCALL_SIGNAL_BLOCK,
				   SYSCALL_SIGNAL_GETHANDLER,
				   SYSCALL_SIGNAL_SETHANDLER};

void setup_syscall();
SyscallReturn syscall(SyscallCode, SyscallParam, int intlock=1);
void register_syscall_handler(SyscallCode, SyscallHandler);



#endif /* HEADERS_SYSCALL_H_ */
