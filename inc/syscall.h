#ifndef OSLAB_SYSCALL_H
#define OSLAB_SYSCALL_H

#include "stdarg.h"
#include <inc/types.h>

#define HZ 100

enum {
    SYS_vprintk = 0,
    SYS_exit,
    SYS_timer,
    SYS_keyboard,
    SYS_display,
    SYS_crash,
    SYS_sleep,
    SYS_wait_intr,
    SYS_getpid,
    SYS_fork,
    SYS_sem_init,
    SYS_sem_destroy,
    SYS_sem_wait,
    SYS_sem_post,
};

void sys_vprintk(const char *ctl, va_list arg);
void sys_timer(void (*)(void));
void sys_keyboard(void (*)(int));
void sys_display(uint8_t *buffer);

void sys_crash() __attribute__((noreturn));

void sys_sleep(uint32_t time);

uint32_t sys_fork();

void sys_exit() __attribute__((noreturn));


void sys_wait_intr();

int sys_getpid();

int sys_sem_init(int sem);

int sys_sem_destroy(int sem);

int sys_sem_wait(int sem);

int sys_sem_post(int sem);

#endif //OSLAB_SYSCALL_H
