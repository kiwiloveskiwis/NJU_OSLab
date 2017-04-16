#include <inc/common.h>
#include <inc/x86.h>

int
abort(const char *fname, int line) {
	/* disable intr to avoid other errors 
		Display error info and stay awaiting */
	disable_interrupt();
	printk("%s:%d: Assertion failed.\n", fname, line);
	while (TRUE) {
		wait_for_interrupt();
	}
}
