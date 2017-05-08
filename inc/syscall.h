#ifndef OSLAB_SYSCALL_H
#define OSLAB_SYSCALL_H

#include "stdarg.h"
#include <inc/types.h>

#define HZ 100

enum {
    SYS_vprintk = 0,
    SYS_timer,
    SYS_keyboard,
    SYS_display,
    SYS_crash,
    SYS_sleep,
};

void sys_vprintk(const char *ctl, va_list arg);
void sys_timer(void (*)(void));
void sys_keyboard(void (*)(int));
void sys_display(uint8_t *buffer);

void sys_crash() __attribute__((noreturn));

void sys_sleep();

#endif //OSLAB_SYSCALL_H
