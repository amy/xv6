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
	static int tick;
	tick++;

	// when the counter hits a ceiling, 
	// edit the return address of the handler 
	// to skip the x/y instruction
	if (tick == 5000) {

		uint info_address = &info;

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

	printf(1, "Traps Performed: XXXX\n");
	printf(1, "Total Elapsed Time: XXXX\n");
	printf(1, "Average Time Per Trap: XXXXX\n");

	exit();
}
