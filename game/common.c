#include <inc/syscall.h>

void printk(const char *ctl, ...) {
    va_list arg;
    va_start(arg, ctl);
    sys_vprintk(ctl, arg);
    va_end(arg);
}

int
abort(const char *fname, int line) {
    /* disable intr to avoid other errors
        Display error info and stay awaiting */
    printk("%s:%d: Assertion failed.\n", fname, line);
    for (;;);   // TODO: SYS_exit or SYS_sleep
}
