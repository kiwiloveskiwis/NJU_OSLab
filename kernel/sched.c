#include <inc/pcb.h>
#include "inc/sched.h"

__attribute__((noreturn))
void sched_process(){ // change to another process
    printk("Scheduling processes\n");
    my_assert(user_pcbs[get_pid()].status != PCB_RUNNING);
    for(int i = 0; i < UPCB_NUM; i++) {
        if(i == get_pid()) continue;
//        if(i < 3) printk("Process %d: status = %d \n", i, user_pcbs[i].status);
        if(user_pcbs[i].status == PCB_RUNNABLE) {
            pcb_exec(&user_pcbs[i]); // no return TODO: test it
            panic("sched_process returned;");
        }
    }

    panic("No runnable process");
}