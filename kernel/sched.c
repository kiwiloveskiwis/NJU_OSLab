#include <inc/pcb.h>
#include "inc/sched.h"

__attribute__((noreturn))
void sched_process(){ // change to another process
    printk("Scheduling processes\n");
    my_assert(user_pcbs[get_pid()].status != PCB_RUNNING);
    for(int i = 0; i < UPCB_NUM; i++) {
        if(i == get_pid()) continue;
        if(user_pcbs[i].status == PCB_RUNNABLE)
            // pcb_exec(i); // no return
            panic(""); // TODO: implement this
    }
    panic("No runnable process");
}