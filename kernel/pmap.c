
#include <inc/memlayout.h>
#include <inc/pmap.h>
#include <inc/string.h>


size_t npages;			// Amount of physical memory (in pages)


extern pde_t entry_pgdir[NPDENTRIES];
struct PageInfo *pages;					// Physical page state array

__attribute__((__aligned__(PGSIZE)))
pte_t user_pgdir[UPDIR_NUM][NPDENTRIES];    //  pgdir no more than pcb

__attribute__((__aligned__(PGSIZE))) // IMPORTANT!!! may cause bugs
static pte_t user_pgtable[USER_DIR_NUM][NPTENTRIES]; // restricted by qemu memory (112 MB for user, 28 * 1024 pages)

// static uint32_t pid = 0;



void pcb_page_init(uint32_t pid) { // copy kernel page dir
    memset(&user_pgdir[pid], 0, sizeof(pte_t) * NPDENTRIES);
    user_pgdir[pid][0] = entry_pgdir[0];  // [0, 4MB)
    user_pgdir[pid][KERNBASE >> PDXSHIFT] = entry_pgdir[KERNBASE >> PDXSHIFT];  // [KERNBASE, KERNBASE+4MB) to PA's [0, 4MB)
}


void alloc_page(uintptr_t va, uint32_t flags, uint32_t pid) {
    // my_assert(pid == get_pid());  // TODO: other cases?
  
    user_pgdir[pid][PDX(va)] = (pde_t) (((uintptr_t) &user_pgtable[pid] - KERNBASE) | flags);
    for (int i = 0; i < NPTENTRIES; ++i) {
        user_pgtable[pid][i] = (pte_t) ((KERN_MEM + (pid << PDXSHIFT)) | i << PTXSHIFT | flags);
        // printk("0x%x\n", user_pgtable[userpg_used][i]);
    }
    printk("%s: Allocated 4MB at 0x%x to 0x%x\n", __func__, va, KERN_MEM + (pid << PDXSHIFT));
}

void pmap_copy(int dest, int src) { // copy to 0x10000000
    printk("curr_pgdir == 0x%x, should be 0x%x \n", rcr3(), user_pgdir[src]);
    // my_assert((pte_t) (rcr3() & ~0x3FF)== (pte_t)user_pgdir[src]);
    printk("0x%x\n", (uintptr_t) &user_pgtable[dest] - KERNBASE);

    uint32_t new_addr = 0x10000000, flags = PTE_P | PTE_W | PTE_U;
    alloc_page(new_addr, flags, dest);

    user_pgdir[src][PDX(new_addr)] = (pde_t) (((uintptr_t) &user_pgtable[dest] - KERNBASE) | PTE_P | PTE_W | PTE_U);
    memcpy((void *) new_addr, (const void *) 0x8000000, PTSIZE); // 4 MB
    user_pgdir[src][PDX(new_addr)] = (pde_t) 0; // clean up
}




