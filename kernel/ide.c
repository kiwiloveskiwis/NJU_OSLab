#include <inc/ide.h>
#include <inc/x86.h>
#include "assert.h"

#define IDE_BSY		0x80
#define IDE_DRDY	0x40
#define IDE_DF		0x20
#define IDE_ERR		0x01


static int
ide_wait_ready(bool check_error) {
    int r;

    while (((r = inb(0x1F7)) & (IDE_BSY|IDE_DRDY)) != IDE_DRDY)
        /* do nothing */;

    if (check_error && (r & (IDE_DF|IDE_ERR)) != 0)
        return -1;
    return 0;
}

_Static_assert(!(SECTSIZE & (SECTSIZE - 1)), "SECTSIZE must be a power of 2!");
int ide_read(uint32_t sec_idx, void *dst, size_t sec_num) {
    my_assert(sec_num <= 256);

    ide_wait_ready(0);

    outb(0x1F2, sec_num);
    outb(0x1F3, sec_idx & 0xFF);
    outb(0x1F4, (sec_idx >> 8) & 0xFF);
    outb(0x1F5, (sec_idx >> 16) & 0xFF);
    outb(0x1F6, 0xE0 | (((sec_idx>>24)&0x0F)));
    outb(0x1F7, 0x20);	// CMD 0x20 means read sector

    for (; sec_num > 0; sec_num--, dst += SECTSIZE) {
        int r;
        if ((r = ide_wait_ready(1)) < 0)
            return r;
        insl(0x1F0, dst, SECTSIZE/4);
    }

    return 0;
}

int ide_write(uint32_t secno, const void *src, size_t sec_num) {

    my_assert(sec_num <= 256);
    ide_wait_ready(0);
    outb(0x1F2, sec_num);
    outb(0x1F3, secno & 0xFF);
    outb(0x1F4, (secno >> 8) & 0xFF);
    outb(0x1F5, (secno >> 16) & 0xFF);
    outb(0x1F6, 0xE0 | ((secno>>24)&0x0F));
    outb(0x1F7, 0x30);	// CMD 0x30 means write sector
    for (; sec_num > 0; sec_num--, src += SECTSIZE) {
        int r;
        if ((r = ide_wait_ready(1)) < 0)
            return r;
        outsl(0x1F0, src, SECTSIZE/4);
    }
    return 0;
}