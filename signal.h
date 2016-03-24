#ifndef XV6_SIGNAL
#define XV6_SIGNAL
// signal.h
// You should define anything signal related that needs to be shared between
// kernel and userspace here

// At a minimum you must define the signal constants themselves
// as well as a sighandler_t type.

struct siginfo_t{
    int signum;
};

typedef void (*sighandler_t)(struct siginfo_t);

#define SIGFPE  11
#define SIGALRM 12
#endif