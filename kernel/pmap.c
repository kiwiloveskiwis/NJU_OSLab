
#include <inc/memlayout.h>
#include <inc/pmap.h>

#define MAX_MEM     0x8000000
#define KERN_MEM    0x1000000
#define USER_MEMSIZE   0x7000000  // 128 - 16 MB

size_t npages;			// Amount of physical memory (in pages)

extern pde_t entry_pgdir[NPDENTRIES];
struct PageInfo *pages;					// Physical page state array

static pte_t user_pgtable[USER_MEMSIZE >> PDXSHIFT][NPTENTRIES];
static uint32_t userpg_used;


void alloc_page(uintptr_t va, uint32_t flags) {
    entry_pgdir[PDX(va)] = (pde_t) (((uintptr_t) &user_pgtable[userpg_used] - KERNBASE) | flags);
    for (int i = 0; i < NPTENTRIES; ++i)
        user_pgtable[userpg_used][i] = (pte_t) ((KERN_MEM + (userpg_used << PDXSHIFT)) | i << PTXSHIFT | flags);
    printk("%s: Allocated 4MB at 0x%x to 0x%x\n", __func__, va, KERN_MEM + (userpg_used << PDXSHIFT),
              userpg_used + 1, USER_MEMSIZE >> PTSHIFT);
    userpg_used++;
}

// Given 'pgdir', a pointer to a page directory, pgdir_walk returns
// a pointer to the page table entry (PTE) for linear address 'va'.
// This requires walking the two-level page table structure.
//
// The relevant page table page might not exist yet.
// If this is true, and create == false, then pgdir_walk returns NULL.
// Otherwise, pgdir_walk allocates a new page table page with page_alloc.
//    - If the allocation fails, pgdir_walk returns NULL.
//    - Otherwise, the new page's reference count is incremented,
//	the page is cleared,
//	and pgdir_walk returns a pointer into the new page table page.
//
// Hint 1: you can turn a Page * into the physical address of the
// page it refers to with page2pa() from kern/pmap.h.
//
// Hint 2: the x86 MMU checks permission bits in both the page directory
// and the page table, so it's safe to leave permissions in the page
// directory more permissive than strictly necessary.
//
// Hint 3: look at inc/mmu.h for useful macros that mainipulate page
// table and page directory entries.
//
pte_t * pgdir_walk(pde_t *pgdir, const void *va, int create) {
    // TODO: add create condition and test it
    struct PageInfo* pg;
    create = false;
    if(!(pgdir[PDX(va)] & PTE_P) && !create) return NULL;

    pte_t* page_vmem = pgdir[PDX(va)] + KERNBASE;
    if(!(page_vmem[PTX(va)] & PTE_P) && !create) return  NULL;
    else return page_vmem + PTX(va);
}



