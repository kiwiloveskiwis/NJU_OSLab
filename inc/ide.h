#ifndef OSLAB_IDE_H
#define OSLAB_IDE_H

#include "types.h"

#define SECTSIZE  512

int ide_read(uint32_t sec_idx, void *dst, size_t sec_num);

int ide_write(uint32_t secno, const void *src, size_t sec_num);

#endif //OSLAB_IDE_H