
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
    // memset(&user_pgdir[pid], 0, sizeof(pte_t) * NPDENTRIES);
    user_pgdir[pid][0] = entry_pgdir[0];  // [0, 4MB)
    user_pgdir[pid][KERNBASE >> PDXSHIFT] = entry_pgdir[KERNBASE >> PDXSHIFT];  // [KERNBASE, KERNBASE+4MB) to PA's [0, 4MB)
}


physaddr_t alloc_page(uintptr_t va, uint32_t flags, uint32_t pid) {
//    int current_pid = get_pid(); // if pid != current_pid, then copy pmap
//    my_assert(current_pid == pid);

    user_pgdir[pid][PDX(va)] = (pde_t) (((uintptr_t) &user_pgtable[pid] - KERNBASE) | flags);
    for (int i = 0; i < NPTENTRIES; ++i) {
        user_pgtable[pid][i] = (pte_t) ((KERN_MEM + (pid << PDXSHIFT)) | i << PTXSHIFT | flags);
        // printk("0x%x\n", user_pgtable[userpg_used][i]);
    }

    printk("%s: PID %d Allocated 4MB at 0x%x to 0x%x\n", __func__, pid, PDX(va) << PDXSHIFT, KERN_MEM + (pid << PDXSHIFT));
    return KERN_MEM + (pid << PDXSHIFT); // Physical addr
}


uint32_t new_addr = 0x10000000;  // Don't overlap kernel and 0x8000000. Otherwise is fine

// Do a complete copy
void pmap_init_and_copy(int dest_pid, int src_pid) {
    pcb_page_init(dest_pid);

    uint32_t flags = PTE_P | PTE_W | PTE_U;
    alloc_page(0x8000000, flags, dest_pid);

    user_pgdir[src_pid][PDX(new_addr)] = (pde_t) (((uintptr_t) &user_pgtable[dest_pid] - KERNBASE) | PTE_P | PTE_W | PTE_U);
    memcpy((void *) new_addr, (const void *) 0x8000000, PTSIZE);    // copy 4 MB to pa 0x1400000
    user_pgdir[src_pid][PDX(new_addr)] = (pde_t) 0;              // clean up
}

void pmap_copy_one_page(int dest_pid, int src_pid, uint32_t va) {
    my_assert((va & (PGSIZE - 1)) == 0);    // aligned

    user_pgtable[dest_pid][PTX(va)] = user_pgtable[src_pid][PTX(va)];
    printk("Copyed one page from %d to %d, va == 0x%x\n", src_pid, dest_pid, va);
}



