#include "assert.h"
#include <inc/memlayout.h>
#include "string.h"
#include <inc/pcb.h>
#include <inc/pmap.h>


static uint32_t curr_pid;
struct PCB user_pcbs[UPCB_NUM];

uint32_t get_pid() {
    return curr_pid;
}

void pcb_init(struct PCB *pcb, uintptr_t esp, uintptr_t eip, uint32_t eflags) {

    pcb->status = PCB_RUNNING;
    pcb->runned_time = 0;

    memset(&pcb->tf, 0, sizeof(pcb->tf));
    pcb->tf.tf_ds = pcb->tf.tf_es = pcb->tf.tf_ss = GD_UD | 3;
    pcb->tf.tf_esp = esp;
    pcb->tf.tf_cs = GD_UT | 3;
    pcb->tf.tf_eip = eip;
    pcb->tf.tf_eflags = eflags;
}

void pcb_exec(struct PCB *pcb) {
    // TODO: lcr3
    pcb->status = PCB_FREE;
    __asm __volatile("movl %0,%%esp\n"
            "\tpopal\n"
            "\tpopl %%es\n"
            "\tpopl %%ds\n"
            "\taddl $0x8,%%esp\n" /* skip tf_trapno and tf_errcode */
            "\tiret"
    : : "g" (&pcb->tf) : "memory");
    panic("iret failed");  /* mostly to placate the compiler */
}


