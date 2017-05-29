#include <inc/pcb.h>
#include "inc/sched.h"

__attribute__((noreturn))
void sched_process(){ // change to another process
    printk("%s: Scheduling processes\n", __FILE__);
    my_assert(user_pcbs[get_pid()].status != PCB_RUNNING);
    for(int i = 0; i < UPCB_NUM; i++) {
        if(i == get_pid()) continue;
//        if(i < 3) printk("Process %d: status = %d \n", i, user_pcbs[i].status);
        if(user_pcbs[i].status == PCB_RUNNABLE) {
            printk("%s: Runnable process: %d\n", __FILE__ ,i);
            pcb_exec(&user_pcbs[i]); // no return
            panic("sched_process returned;");
        } else if(user_pcbs[i].status == PCB_SLEEPING
                  && user_pcbs[i].wakeup_time < sys_runned_time
                    && user_pcbs[i].wait_sem == NOT_WAITING_SEM ) {
            pcb_exec(&user_pcbs[i]); // no return
        }
    }

    panic("No runnable process");
}