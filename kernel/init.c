#define TRUE 1
#define FALSE 0
#include <inc/common.h>
#include <inc/x86.h>
#include <inc/game.h>
#include <inc/timer.h>
#include <inc/pcb.h>
#include <assert.h>
#include <memory.h>
#include <inc/ide.h>
#include <inc/elf.h>
#include "trap.h"

extern void init_intr();
extern void main_loop();

#define SECTCOUNT 1
#define E_SUCCESS 0
uintptr_t userprog_load(uint32_t offset) {
	printk("Loading...%s\n", __func__);
	uint8_t header[SECTSIZE * SECTCOUNT];
	my_assert(ide_read(header, offset, SECTSIZE * SECTCOUNT) == E_SUCCESS);
	struct Elf *elfheader = (struct Elf *) header;
	my_assert(elfheader->e_magic == 0x464C457FU);
	my_assert(elfheader->e_phoff + elfheader->e_phnum * sizeof(struct Proghdr) <= SECTSIZE * SECTCOUNT);
	struct Proghdr *ph = (struct Proghdr *) (header + elfheader->e_phoff);
	for (int phnum = elfheader->e_phnum; phnum > 0; --phnum, ++ph)
		if (ph->p_type == 1) { // ELF_PROG_LOAD
			printk("ph->p_pa = %x\n", ph->p_pa);
			my_assert(ph->p_pa >= 0x1000000);         // don't overlap kernel
			my_assert(ide_read((void *) ph->p_pa, offset + ph->p_offset, ph->p_filesz) == E_SUCCESS);
			memset((void *) (ph->p_pa + ph->p_filesz), 0, ph->p_memsz - ph->p_filesz);
		}
	printk("Loading finished!, e_entry(va) is 0x%x\n", __func__, elfheader->e_entry);
	return elfheader->e_entry; // would be eip of pcb_init
}

void kernel_init() {
    printk("\nKernel init\n");
	init_serial();
	init_timer();
	init_intr();
	trap_init();

	struct PCB userprog;
	pcb_init(&userprog, 0x8048000, userprog_load(300 * SECTSIZE), 2 | FL_IF);
	pcb_exec(&userprog);
} // ﻿0x1000000 = 2^24   2^9*2^8
