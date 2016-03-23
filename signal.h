// signal.h
#ifndef XV6_SIGNAL
#define XV6_SIGNAL

struct siginfo_t{
    int signum;
};

typedef void (*sighandler_t)(struct siginfo_t);

#define SIGFPE  11
#define SIGALRM 12

#endif