///////////////////////////
///////// STAGE 1 /////////
///////////////////////////

/**
 * register_signal_handler
 */

signal.h
	siginfo_t
	sighandler_t
	SIGFPE
	SIGALRM

sysproc.c
	int sys_register_signal_handler(void) {...}

syscall.c
	extern int sys_register_signal_handler(void);

syscall.h
	#define SYS_register_signal_handler 23

proc.h
	int register_signal_handler(int signum, sighandler_t handler); // prototype
	struct proc {
	  sighandler_t signal_handlers[2];      // Array of signal handler function pointers
	}

proc.c
	int register_signal_handler(int signum, sighandler_t handler) {...}


usys.S
	SYSCALL(register_signal_handler)

user.h
	int register_signal_handler(int, void*);

trap.c
	case T_DIVIDE:
    tf->esp = tf->esp-24; 
    tf->eip = (uint)(proc->signal_handlers[SIGFPE]);
    break;

/**
 * signal
 */

ulib.c
	int signal (int signum, void* handler) {...}

user.h
	int signal(int, void*);

/**
 * alarm
 */



