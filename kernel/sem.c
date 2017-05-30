#include <inc/syscall.h>
#include <inc/assert.h>
#include <inc/pcb.h>
#include <inc/sched.h>


#define SEM_POOL_SIZE 3

struct semaphore {
//    spinlock_t                lock;
    unsigned int             count;
//    struct list_head        wait_list;
};

struct semaphore sem_pool[SEM_POOL_SIZE];
extern void sched_process() __attribute__((noreturn));


int sys_sem_open(int sem, int value) {
    my_assert((sem >= 0 || sem < SEM_POOL_SIZE) && value > 0);
    sem_pool[sem].count = value;
    return sem;
}

int sys_sem_destroy(int sem) {
    my_assert(sem >= 0 || sem < SEM_POOL_SIZE);
    sem_pool[sem].count = 0;
    return E_SUCCESS;
}

int sys_sem_wait(int sem) {
    my_assert(sem >= 0 || sem < SEM_POOL_SIZE);
    if (sem_pool[sem].count == 0) {
        if(user_pcbs[get_pid()].status == PCB_SLEEPING) return E_INVALID; // already sleeping
        user_pcbs[get_pid()].wait_sem = sem;
        user_pcbs[get_pid()].status = PCB_SLEEPING;
        sched_process();       // no return
    } // else
    sem_pool[sem].count--;
    return E_SUCCESS;
}


int sys_sem_post(int sem) {
    my_assert(sem >= 0 || sem < SEM_POOL_SIZE);
    sem_pool[sem].count++;
    return E_SUCCESS;
}