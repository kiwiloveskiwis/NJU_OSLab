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


void pcb_init_p0(struct PCB *pcb, uintptr_t esp, uintptr_t eip, uint32_t eflags) {
    for(int i = 0; i < UPCB_NUM; i++) { // set other pcbs
        user_pcbs[i].status = PCB_FREE;
        user_pcbs[i].runned_time = 0;
        user_pcbs[i].pid = i;
    }

    pcb->status = PCB_RUNNING;
    pcb->runned_time = 0;
    load_updir(0);

    memset(&pcb->tf, 0, sizeof(pcb->tf));
    pcb->tf.tf_ds = pcb->tf.tf_es = pcb->tf.tf_ss = GD_UD | 3;
    pcb->tf.tf_esp = esp;
    pcb->tf.tf_cs = GD_UT | 3;
    pcb->tf.tf_eip = eip;
    pcb->tf.tf_eflags = eflags;
}



void pcb_exec(struct PCB *pcb) {
    curr_pid = pcb->pid;
    load_updir(pcb->pcb_pgdir);
    pcb->status = PCB_RUNNING;

    extern struct Taskstate pts;
    pts.ts_esp0 = (uintptr_t) pcb + sizeof(struct PCB);

    // my_assert(user_pcbs[get_pid()].pcb_pgdir == pcb->pcb_pgdir);

    __asm __volatile("movl %0,%%esp\n"
            "\tpopal\n"
            "\tpopl %%es\n"   // es = 0x23
            "\tpopl %%ds\n"     // ds = 0x23
            "\taddl $0x8,%%esp\n" /* skip tf_trapno and tf_errcode */
            "\tiret"            // [esp] = 0x4ec83cf
    : : "g" (&pcb->tf) : "memory");

    panic("iret failed");
}

