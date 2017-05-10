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
    printk("Sys sleeping\n");
    user_pcbs[get_pid()].status = PCB_SLEEPING;
    user_pcbs[get_pid()].wakeup_time = time;
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

// returns child pid for parent, 0 for children and -1 for error
uint32_t sys_fork() {
    int new_pid = 0, old_pid = get_pid();
    for(new_pid = 0; new_pid < UPCB_NUM; new_pid++)
        if((new_pid != old_pid) && user_pcbs[new_pid].status == PCB_FREE) break;
    if(new_pid == UPCB_NUM) panic("No Free PCB");

    pcb_page_init(new_pid);

    memcpy(&user_pcbs[new_pid], &user_pcbs[old_pid], sizeof(struct PCB));

    user_pcbs[new_pid].tf.tf_regs.reg_eax = 0;
    user_pcbs[new_pid].pid = new_pid; // avoid overlap
    pmap_copy(new_pid, old_pid);

    printk("%s: original_pid = %d, new_pid = %d\n", __func__, old_pid, new_pid);
    return new_pid; //
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
        default:
            return (uint32_t) -1;
    }
}
