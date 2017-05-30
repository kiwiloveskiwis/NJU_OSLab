//
// Created by 卢以宁 on 5/7/17.
//

#ifndef OSLAB_PMAP_H
#define OSLAB_PMAP_H

#include "types.h"
#include "mmu.h"
#include "common.h"
#include "memlayout.h"
#include "x86.h"
#include <inc/pcb.h>



#define MAX_MEM     0x8000000
#define KERN_MEM    0x1000000
#define USER_MEMSIZE   (MAX_MEM - KERN_MEM)  // 128 - 16 MB
#define USER_DIR_NUM (USER_MEMSIZE >> PTSHIFT)
#define UPDIR_NUM 64

extern size_t npages;
extern struct PageInfo *pages;
extern pte_t user_pgdir[UPDIR_NUM][NPDENTRIES];

static inline void load_updir(int pid) {
    uint32_t dir_addr = ((uint32_t)user_pgdir[pid]) - KERNBASE;
    // lcr3((uint32_t)entry_pgdir - KERNBASE);
    lcr3(dir_addr);
}


void pcb_page_init(uint32_t pid);

/* This macro takes a kernel virtual address -- an address that points above
 * KERNBASE, where the machine's maximum 256MB of physical memory is mapped --
 * and returns the corresponding physical address.  It panics if you pass it a
 * non-kernel virtual address.
 */
#define PADDR(kva) _paddr(__FILE__, __LINE__, kva)

static inline physaddr_t
_paddr(const char *file, int line, void *kva)
{
    if ((uint32_t)kva < KERNBASE)
        printk(file, line, "PADDR called with invalid kva %08lx", kva);
    return (physaddr_t)kva - KERNBASE;
}

/* This macro takes a physical address and returns the corresponding kernel
 * virtual address.  It panics if you pass an invalid physical address. */
#define KADDR(pa) _kaddr(__FILE__, __LINE__, pa)

static inline void*
_kaddr(const char *file, int line, physaddr_t pa)
{
    if (PGNUM(pa) >= npages)
        printk(file, line, "KADDR called with invalid pa %08lx", pa);
    return (void *)(pa + KERNBASE);
}

static inline physaddr_t
page2pa(struct PageInfo *pp)
{
    return (pp - pages) << PGSHIFT;
}

static inline struct PageInfo* pa2page(physaddr_t pa) {
    if (PGNUM(pa) >= npages)
        printk("pa2page called with invalid pa");
    return &pages[PGNUM(pa)];
}


void pmap_init_and_copy(int dest, int src);

static inline void*
page2kva(struct PageInfo *pp)
{
    return KADDR(page2pa(pp));
}

//pte_t *pgdir_walk(pde_t *pgdir, const void *va, int create);


physaddr_t alloc_page(uintptr_t va, uint32_t flags, uint32_t pid);

void pmap_copy_one_page(int dest_pid, int src_pid, uint32_t va);

#endif //OSLAB_PMAP_H
