#include <inc/pcb.h>
#include <inc/syscall.h>
#include "inc/sched.h"

__attribute__((noreturn))
void sched_process(){ // change to another process
    int original_pid = get_pid();
//    printk("%s: Scheduling processes. curr_pid = %d\n", __FILE__, original_pid);
    my_assert(user_pcbs[get_pid()].status != PCB_RUNNING);
    while(true) {
        sys_runned_time++;
        for (int round_robin = 0; round_robin < UPCB_NUM; round_robin++) {
            int i = (round_robin + original_pid + 1) % UPCB_NUM;
            if (i == original_pid) continue;
            if (user_pcbs[i].status == PCB_RUNNABLE) {
//                printk("%s: Runnable process: %d\n", __FILE__, i);
                pcb_exec(&user_pcbs[i]); // no return
                panic("sched_process returned;");
            } else if (user_pcbs[i].status == PCB_SLEEPING && user_pcbs[i].wakeup_time <= sys_runned_time) {

                if (user_pcbs[i].wait_sem == NOT_WAITING_SEM) {
                    pcb_exec(&user_pcbs[i]); // no return
                } else if (sys_sem_wait(user_pcbs[i].wait_sem) == E_SUCCESS) {
                    user_pcbs[i].wait_sem = NOT_WAITING_SEM;
                    pcb_exec(&user_pcbs[i]);
                }

            }
        }
        printk("%s %d: No runnable process: looping.", __FILE__, __LINE__);
    }
    my_assert(0);
}