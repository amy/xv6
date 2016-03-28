#include "types.h"
#include "stat.h"
#include "user.h"
#include "signal.h"

// Reference: https://piazza.com/class/ijkf1amk1f44hp?cid=166

void dummy(void)
{
	printf(1, "TEST FAILED: this should never execute.\n");
}

void handle_signal(siginfo_t info)
{
	// this handler executes repeatedly
	static int trapNum;
	trapNum++;

	// when the counter hits a ceiling, 
	// edit the return address of the handler (add 4) 
	// to skip the x/y instruction (dereference and add 4)
	if (trapNum == 1000000) {

		//siginfo_t * info_address = &(info.signum);
		int * info_address = &(info.signum) + 4;
		(*info_address) += 4;

	}

}

int main(int argc, char *argv[])
{

	int x = 5;
	int y = 0;

	int a = uptime();

	signal(SIGFPE, handle_signal);

	x = x / y;

	int b = uptime();

	printf(1, "Traps Performed: 1000000\n");
	printf(1, "Total Elapsed Time: %d\n", (b-a)); // How many ticks it takes to run 100,000 traps
	printf(1, "Average Time Per Trap: %d\n", (b-a)); // in microticks, microtick = 1/1millionth of a tick

	exit();
}
