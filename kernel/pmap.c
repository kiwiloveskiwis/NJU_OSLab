
#include <inc/memlayout.h>
#include <inc/pmap.h>



size_t npages;			// Amount of physical memory (in pages)


extern pde_t entry_pgdir[NPDENTRIES];
struct PageInfo *pages;					// Physical page state array

__attribute__((__aligned__(PGSIZE)))
pte_t user_pgdir[UPDIR_NUM][NPDENTRIES];    //  pgdir no more than pcb

__attribute__((__aligned__(PGSIZE))) // IMPORTANT!!! may cause bugs
static pte_t user_pgtable[USER_DIR_NUM][NPTENTRIES]; // restricted by qemu memory (112 MB for user)
static uint32_t userpg_used[UPCB_NUM];


// page map for main process
void mem_init_zero() {
    uint32_t flags = PTE_P | PTE_W | PTE_U;
    pcb_page_init(user_pcbs[0].pid); // save in process 0
    // panic(" Stop. ");
    alloc_page(0x8048000, flags, 0);
}

void pcb_page_init(uint32_t pid) { // copy kernel page dir
    user_pgdir[pid][0] = entry_pgdir[0];  // [0, 4MB)
    user_pgdir[pid][KERNBASE >> PDXSHIFT] = entry_pgdir[KERNBASE >> PDXSHIFT];  // [KERNBASE, KERNBASE+4MB) to PA's [0, 4MB)
}


void alloc_page(uintptr_t va, uint32_t flags, uint32_t pid) {
    my_assert(pid == get_pid());  // TODO: other cases?
    int32_t used = userpg_used[pid]; // 0 ~ 27
    user_pgdir[pid][PDX(va)] = (pde_t) (((uintptr_t) &user_pgtable[used] - KERNBASE) | flags);
    for (int i = 0; i < NPTENTRIES; ++i) {
        user_pgtable[used][i] = (pte_t) ((KERN_MEM + (used << PDXSHIFT)) | i << PTXSHIFT | flags);
        // printk("0x%x\n", user_pgtable[userpg_used][i]);
    }
    printk("%s: Allocated 4GB at 0x%x to 0x%x\n", __func__, va, KERN_MEM + (used << PDXSHIFT));
    userpg_used[pid]++;
}




