#include <inc/syscall.h>
#include <inc/trap.h>
#include <inc/string.h>
#include <inc/video.h>
#include <inc/cpu.h>
#include <inc/memlayout.h>
#include "trap.h"

void sys_vprintk(const char *ctl, va_list arg) {
    extern void vfprintf(void (*)(char), const char *, va_list);
    extern void serial_printc(char);
    vfprintf(serial_printc, ctl, arg);
}

void sys_timer(void (*handler)(void)) {
    do_timer = handler;
}

void sys_keyboard(void (*handler)(int)) {
    do_keyboard = handler;
}

void sys_display(uint8_t *buffer) {
    memcpy(KERNBASE + VMEM_ADDR, buffer, SCR_SIZE);
}

void sys_sleep() {
    enable_interrupt();
    wait_for_interrupt();
    disable_interrupt();
}

__attribute__((noreturn)) void sys_crash() {
    for (;;) __asm __volatile("cli; hlt");
}

uint32_t syscall_handler(struct Trapframe *tf) {
#define arg1 tf->tf_regs.reg_edx
#define arg2 tf->tf_regs.reg_ecx
#define arg3 tf->tf_regs.reg_ebx
#define arg4 tf->tf_regs.reg_edi
#define arg5 tf->tf_regs.reg_esi
    switch (tf->tf_regs.reg_eax) {  // syscall number
        case SYS_vprintk:
            sys_vprintk((const char *) arg1, (va_list) arg2);
            return 0;
        case SYS_timer:
            sys_timer((void (*)(void)) arg1);
            return 0;
        case SYS_keyboard:
            sys_keyboard((void (*)(int)) arg1);
            return 0;
        case SYS_display:
            sys_display((uint8_t *) arg1);
            return 0;
        case SYS_sleep:
            sys_sleep();
            return 0;
        case SYS_crash:
            sys_crash();
            return 0; // never exec
        default:
            return (uint32_t) -1;
    }
}
