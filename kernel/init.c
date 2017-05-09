#define TRUE 1
#define FALSE 0
#include <inc/common.h>
#include <inc/x86.h>
#include <inc/timer.h>
#include <inc/assert.h>
#include <inc/pcb.h>
#include <assert.h>
#include <memory.h>
#include <inc/ide.h>
#include <inc/elf.h>
#include "trap.h"

extern void pic_init();
extern void main_loop();

#define SECTCOUNT 1
#define USER_START 0x8048000

uintptr_t userprog_load(uint32_t offset) {
	printk("Loading...%s\n", __func__);
	uint8_t header[SECTSIZE * SECTCOUNT];
	ide_read(header, offset, SECTSIZE * SECTCOUNT);
	struct Elf *elfheader = (struct Elf *) header;
	my_assert(elfheader->e_magic == 0x464C457FU);
	my_assert(elfheader->e_phoff + elfheader->e_phnum * sizeof(struct Proghdr) <= SECTSIZE * SECTCOUNT);

	struct Proghdr *ph = (struct Proghdr *) (header + elfheader->e_phoff);
	load_updir(get_pid());
	for (int phnum = elfheader->e_phnum; phnum > 0; --phnum, ++ph)
		if (ph->p_type == 1) { // ELF_PROG_LOAD
			printk("%s: ph->p_pa = %x\n", __func__, ph->p_pa);
			my_assert(ph->p_pa >= 0x1000000);         // don't overlap kernel
			ide_read((void *) ph->p_pa, offset + ph->p_offset, ph->p_filesz);
			memset((void *) (ph->p_pa + ph->p_filesz), 0, ph->p_memsz - ph->p_filesz);
		}
	// printk("Loading finished!, e_entry(va) is 0x%x\n", __func__, elfheader->e_entry); // ﻿0xF0101C98
	return elfheader->e_entry; // would be eip of pcb_init
}




void kernel_init() {
    printk("Kernel init\n");
	init_serial();
	init_timer();
	pic_init();
	trap_init();
	mem_init();

	struct PCB userprog;
	pcb_init(&userprog, USER_START, userprog_load(300 * SECTSIZE), 2 | FL_IF);

	pcb_exec(&userprog);
} // ﻿0x1000000 = 2^24   2^9*2^8
