#include <inc/syscall.h>
#include <inc/trap.h>
#include <inc/string.h>
#include <inc/video.h>
#include <inc/cpu.h>
#include <inc/memlayout.h>
#include <inc/pcb.h>
#include <inc/sched.h>
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

void sys_sleep(uint32_t time) {
    printk("sys_sleeping, pid = %d\n", get_pid());
    user_pcbs[get_pid()].status = PCB_SLEEPING;
    user_pcbs[get_pid()].wakeup_time = sys_runned_time + time;
    sched_process();
    // TODO
}

__attribute__((noreturn))
void sys_exit() {
    printk("%s: pid %d exited gracefully.\n", __func__, get_pid());
    user_pcbs[get_pid()].status = PCB_FREE;
    sched_process();
}

void sys_wait_intr() {
    enable_interrupt();
    wait_for_interrupt();
    disable_interrupt();
}
//﻿0x8048a8b

// returns child pid for parent, 0 for children and -1 for error
uint32_t sys_fork() {
    printk("Fork.\n");
    int new_pid = 0, old_pid = get_pid();
    for(new_pid = 0; new_pid < UPCB_NUM; new_pid++)
        if((new_pid != old_pid) && user_pcbs[new_pid].status == PCB_FREE) break;
    if(new_pid == UPCB_NUM) panic("No Free PCB");


    memcpy(&user_pcbs[new_pid], &user_pcbs[old_pid], sizeof(struct PCB));
    user_pcbs[new_pid].pid = new_pid; // avoid overlap
    user_pcbs[new_pid].status = PCB_RUNNABLE;
    user_pcbs[new_pid].tf.tf_regs.reg_eax = 0; // return 0
    pmap_copy(new_pid, old_pid);

    printk("%s: original_pid = %d, new_pid = %d\n", __func__, old_pid, new_pid);
    printk("Original eip == 0x%x\n", user_pcbs[old_pid].tf.tf_eip); // ﻿Original eip == 0x80488D9 is wrong
    return new_pid; //
}

__attribute__((noreturn)) void sys_crash() {
    for (;;) __asm __volatile("cli; hlt");
}

uint32_t syscall_handler(struct Trapframe *tf) {
    disable_interrupt();
#define arg1 tf->tf_regs.reg_edx
#define arg2 tf->tf_regs.reg_ecx
#define arg3 tf->tf_regs.reg_ebx
#define arg4 tf->tf_regs.reg_edi
#define arg5 tf->tf_regs.reg_esi
    switch (tf->tf_regs.reg_eax) {  // syscall number
        case SYS_vprintk:
            sys_vprintk((const char *) arg1, (va_list) arg2);
            return 0;
        case SYS_exit:
            sys_exit();
            return 0;
        case SYS_fork:
            return sys_fork();
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
            sys_sleep((uint32_t) arg1);
            return 0;
        case SYS_wait_intr:
            sys_wait_intr();
            return 0;
        case SYS_crash:
            printk("Crashed.\n");
            sys_crash();
            return 0; // never exec
        case SYS_getpid:
            return get_pid();
        case SYS_sem_open:
            return sys_sem_open((uint32_t) arg1, (uint32_t) arg2);
        case SYS_sem_destroy:
            return sys_sem_destroy((uint32_t) arg1);
        case SYS_sem_wait:
            return sys_sem_wait((uint32_t) arg1);
        case SYS_sem_post:
            return sys_sem_post((uint32_t) arg1);
        default:
            return (uint32_t) -1;
    }
}
