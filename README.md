Things I did

Stage 1:

signal.h
	> SIGFPE & SIGALRM constants
	> siginfo_t struct
	> signalhandler function declaration

proc.c
proc.h

Signal Registration
	Within a process, if you are registering handler to a signum that's already been registered,
	it just gets overwritten with the new handler.