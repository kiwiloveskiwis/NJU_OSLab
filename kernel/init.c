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
#include <inc/syscall.h>
#include "trap.h"
#include "fs.h"

extern void pic_init();
extern void main_loop();

#define SECTCOUNT 1
#define USER_START 0x8048000

uintptr_t userprog_load(int pid, char *filename) {
	my_assert(get_pid()==pid);
    pcb_page_init(pid);

    alloc_page(USER_START, PTE_P | PTE_W | PTE_U, 0);
	printk("Loading...%s\n", __func__);

	int fd = fs_open(filename, O_RDONLY);
	if (fd < 0) panic("fs_open failed: %d\n", fd);

	uint8_t header[SECTSIZE * SECTCOUNT];
    my_assert(fs_read(fd, header, SECTSIZE * SECTCOUNT) == SECTSIZE * SECTCOUNT);

	struct Elf *elfheader = (struct Elf *) header;
	my_assert(elfheader->e_magic == 0x464C457FU);
	my_assert(elfheader->e_phoff + elfheader->e_phnum * sizeof(struct Proghdr) <= SECTSIZE * SECTCOUNT);

	struct Proghdr *ph = (struct Proghdr *) (header + elfheader->e_phoff);
	load_updir(pid);
    for (int phnum = elfheader->e_phnum; phnum > 0; --phnum, ++ph) if (ph->p_type == 1) { // ELF_PROG_LOAD
        my_assert(ph->p_pa >= 0x8000000 && ph->p_pa + ph->p_memsz <= 0x8400000);
        if (ph->p_filesz) {
            fs_lseek(fd, ph->p_offset, SEEK_SET);
            my_assert(fs_read(fd, (void *) ph->p_pa, ph->p_filesz) == ph->p_filesz);
        }
        memset((void *) (ph->p_pa + ph->p_filesz), 0, ph->p_memsz - ph->p_filesz);
    }
	printk("Loading finished, e_entry(va) is 0x%x\n", __func__, elfheader->e_entry); // ﻿0xF01039E8
	my_assert(!fs_close(fd));
	return elfheader->e_entry; // would used by pcb_init_p0 as eip
}
// b *﻿0xf01014f7



void kernel_init() {
    printk("Kernel init\n");
	init_serial();
	init_timer();
	pic_init();
	trap_init();
    fs_init();

	pcb_init_p0(&user_pcbs[0], USER_START, userprog_load(0, "user.bin"), 2 | FL_IF);

	pcb_exec(&user_pcbs[0]); // ﻿0xf010151a
} // ﻿0x1000000 = 2^24   2^9*2^8
