#ifndef OSLAB_PCB_H
#define OSLAB_PCB_H

#include "types.h"
#include "trap.h"
#include "pmap.h"

#define KSTACK_SIZE 4096
#define UPCB_NUM 64


enum {
    PCB_FREE = 0,
    PCB_SLEEPING,
    PCB_RUNNABLE,
    PCB_RUNNING,
    PCB_NOT_RUNNABLE
};

struct PCB {
    uint32_t pid;
    uint32_t parent_pid;
    uint32_t status;
    uint32_t runned_time;
    uint32_t wakeup_time;
    uintptr_t pcb_pgdir;
    struct Trapframe tf;
    uint8_t kstack[KSTACK_SIZE];

};



extern struct PCB user_pcbs[UPCB_NUM];

/* 进程在内核状态下的栈
进程的标识符
进程的父进程的标识符
进程的状态（正在执行、可以执行、正在休眠、不可执行等）
进程已经运行的时间片数量
进程的页目录表的地址
 */

uint32_t get_pid();

void pcb_init();

void pcb_enter(struct PCB *pcb, uintptr_t esp, uintptr_t eip, uint32_t eflags);

void pcb_exec(struct PCB *pcb);
#endif //OSLAB_PCB_H
