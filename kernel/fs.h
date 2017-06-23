#ifndef OSLAB_FS_H
#define OSLAB_FS_H

#include "inc/ide.h"

#define FILENAME_LENGTH 24
#define ENTRY_COUNT (SECTSIZE / sizeof(DirEntry))
#define N_INODE_DATABLOCK (SECTSIZE / sizeof(uint32_t) - 1)
#define INODE_CAPACITY (N_INODE_DATABLOCK * SECTSIZE)
#define BITMAP_SIZE (SECTSIZE * 8)

/**
 * Fs structure: (data = 2MB)
 * +------------+--------+-------+------+
 * | bootloader | bitmap |  dir  | data |
 * +------------+--------+-------+------+
 */

#define FSOFFSET_BOOTLOADER 0
#define FSOFFSET_BITMAP (FSOFFSET_BOOTLOADER + 1) // 1
#define FSLENGTH_BITMAP (BITMAP_SIZE / SECTSIZE / 8) // 1
#define FSOFFSET_DIR (FSOFFSET_BITMAP + FSLENGTH_BITMAP) // 2
#define FSOFFSET_DATA (FSOFFSET_DIR + 1) // 3
#define FSOFFSET_END (FSOFFSET_DATA + BITMAP_SIZE) // 8 * 512 + 3
_Static_assert(FSOFFSET_END == 4099, "Unexpected img size!");

extern const char filename_kernel[];

typedef struct {
    char fileName[FILENAME_LENGTH];
    uint32_t fileSize, inodeOffset;
} DirEntry, Dir[ENTRY_COUNT];
_Static_assert(sizeof(Dir) == SECTSIZE, "Unexpected Dir struct!");

typedef struct {
    // We don't need random access, therefore use a linked list
    uint32_t dataBlocks[N_INODE_DATABLOCK], next;
} INode;
_Static_assert(sizeof(INode) == SECTSIZE, "Unexpected INode struct!");



typedef uint32_t Bitmap[BITMAP_SIZE / 32];

void fs_init();
int fs_open(const char *pathname, int flags);
int fs_read(int fd, void *buf, int len);
int fs_write(int fd, const void *buf, int len);
int fs_lseek(int fd, int offset, int whence);
int fs_close(int fd);

#endif //OSLAB_FS_H
