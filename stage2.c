#include "types.h"
#include "user.h"
#include "signal.h"

volatile int flag = 0;

void dummy(void)
{
    printf(1, "TEST FAILED: this should never execute.\n");
}

void handle_signal(siginfo_t info)
{
    __asm__ ("movl $0x0,%ecx\n\t");
    flag = 1;
} 

int main(void)
{
    register int ecx asm ("%ecx");
    
    signal(SIGALRM, handle_signal);
    //signal(SIGFPE, handle_signal);

    ecx = 5;

    alarm(1);
    //int x = 5;
    //int y = 0;
    //x = x / y;

    while(!flag);

    printf(1, "DOESN'T HIT HERE");

    if (ecx == 5)
        printf(1, "TEST PASSED: Final value of ecx is %d...\n", ecx);
    else
        printf(1, "TEST FAILED: Final value of ecx is %d...\n", ecx);

    exit();
}
