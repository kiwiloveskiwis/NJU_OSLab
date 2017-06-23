#include <inc/x86.h>
#include <inc/elf.h>
#include <kernel/fs.h>

#define SECTSIZE	512

void readsect(void*, uint32_t);

static inline void memset(void *dest, int c, size_t s) {
    while (s--) *(uint8_t *) dest++ = (uint8_t) c;
}

void
bootmain(void) {
    INode inode;
    uint8_t header[SECTSIZE];
    int index_inode = 0;
    readsect(&inode, FSOFFSET_DATA);        // assert: dir[0].inodeOffset = 0
    readsect(header, FSOFFSET_DATA + 1);    // assert #2: ELF header at block #1

    struct Elf *elfheader = (struct Elf *) header;
//    if (elfheader->e_magic != ELF_MAGIC) goto bad; // is this a valid ELF?

    // load each program segment (ignores ph flags)
    struct Proghdr *ph = (struct Proghdr *) ((uint8_t *) header + elfheader->e_phoff);

    void *eph = ph + elfheader->e_phnum;
    for (; (void*)ph < eph; ph++) if (ph->p_memsz) {
        // assert (inode_idx <= i) ;
        while (index_inode < ph->p_offset / SECTSIZE / N_INODE_DATABLOCK) {
            readsect(&inode, FSOFFSET_DATA + inode.next);
            ++index_inode;
        }
        int i = ph->p_offset / SECTSIZE % N_INODE_DATABLOCK;
        int count = 0;
        while (count < ph->p_filesz) {
            readsect((void *) (ph->p_pa + count), FSOFFSET_DATA + inode.dataBlocks[i]);
            if (++i == N_INODE_DATABLOCK) {
                i = 0;
                readsect(&inode, FSOFFSET_DATA + inode.next);
                ++index_inode;
            }
            count += SECTSIZE;
        }
        memset((void *) (ph->p_pa + ph->p_filesz), 0, ph->p_memsz - ph->p_filesz);
    }

    ((void (*)()) (elfheader->e_entry))(); // does not return!

    while (1)
        /* do nothing */;
}



void
waitdisk(void) {
    // wait for disk reaady
    while ((inb(0x1F7) & 0xC0) != 0x40)
        /* do nothing */;
}

void
readsect(void *dst, uint32_t offset) {
    // wait for disk to be ready
    waitdisk();

    outb(0x1F2, 1);		// count = 1
    outb(0x1F3, offset);
    outb(0x1F4, offset >> 8);
    outb(0x1F5, offset >> 16);
    outb(0x1F6, (offset >> 24) | 0xE0);
    outb(0x1F7, 0x20);	// cmd 0x20 - read sectors

    // wait for disk to be ready
    waitdisk();

    // read a sector
    insl(0x1F0, dst, SECTSIZE/4);
}
