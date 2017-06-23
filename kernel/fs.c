#include <inc/syscall.h>
#include "assert.h"
#include "string.h"
#include "kernel/fs.h"
#include "inc/pcb.h"
#include "fs.h"

#define FDPOOL_SIZE (ENTRY_COUNT * UPCB_NUM)

#define IDE_READ(secno, dest) ide_read((secno) + FSOFFSET_DATA, dest, 1)
#define IDE_WRITE(secno, dest) ide_write((secno) + FSOFFSET_DATA, dest, 1)

#define GARBAGE 0
#define USEFUL  1
#define NEED_WASHING 2

Bitmap bitmap;
Dir dir;

uint32_t state_bitmap, state_dir;

struct {
    int index;
    size_t offset;
    uint8_t buffer[SECTSIZE];
    INode inode;
    uint32_t buffer_idx, inode_idx, buffer_state, inode_state;
    bool opened, writable;
} fd_pool[FDPOOL_SIZE];

void fs_init() {
    state_bitmap = state_dir = USEFUL;
    ide_read(FSOFFSET_DIR, &dir, 1);
    ide_read(FSOFFSET_BITMAP, &bitmap, FSLENGTH_BITMAP);
}

int fs_flush() {
    if (state_dir == NEED_WASHING) ide_write(FSOFFSET_DIR, &dir, 1);
    if (state_bitmap == NEED_WASHING) ide_write(FSOFFSET_BITMAP, &bitmap, FSLENGTH_BITMAP);
    state_bitmap = state_dir = USEFUL;
    return 0;
}

uint32_t find_empty_inode() {
    my_assert(state_bitmap != GARBAGE);
    for (int i = 0; i < BITMAP_SIZE; ++i) {
        uint32_t j = ~bitmap[i], bit_count = 0;
        if (j == 0) continue;
        while(!(j & 0x1)) {
            j >>= 1;
            bit_count += 1;
        }
        state_bitmap = NEED_WASHING;
        bitmap[i] |= 1 << bit_count;
        return (i << 5) + bit_count;
    }
    panic(0, "No more empty inode available");
}

int fd_inode_flush(int fd) {
    if (fd_pool[fd].inode_state == NEED_WASHING) {
        fd_pool[fd].inode_state = USEFUL;
        IDE_WRITE(fd_pool[fd].inode_idx, &fd_pool[fd].inode);
    }
    return 0;
}

void fd_set_inode(int fd) {
    fd_pool[fd].inode_idx = dir[fd_pool[fd].index].inodeOffset = (uint32_t) find_empty_inode();
    state_dir = NEED_WASHING;
    memset(&fd_pool[fd].inode, 0, SECTSIZE);
    fd_pool[fd].inode_state = NEED_WASHING;
}

int fd_alloc_next_inode(int fd) {
    fd_pool[fd].inode_state = NEED_WASHING;
    fd_pool[fd].inode.next = (uint32_t) find_empty_inode();
    fd_inode_flush(fd);
    fd_pool[fd].inode_state = NEED_WASHING;
    fd_pool[fd].inode_idx = fd_pool[fd].inode.next;
    memset(&fd_pool[fd].inode, 0, SECTSIZE);
    return 0;
}

int fd_inode_state_init(int fd) {
    my_assert(fd_pool[fd].offset % INODE_CAPACITY == 0);
//    my_assert(fd_pool[fd].inode_state == GARBAGE);

    if (fd_pool[fd].offset == 0) {
        if (dir[fd_pool[fd].index].inodeOffset == -1) {
            fd_set_inode(fd);
            return 0;
        }
        fd_pool[fd].inode_state = USEFUL;
        IDE_READ(fd_pool[fd].inode_idx = dir[fd_pool[fd].index].inodeOffset, &fd_pool[fd].inode);
        return 0;
    }
    // already started reading
    if (fd_pool[fd].inode.next == 0) {
        fd_alloc_next_inode(fd);
        return 0;
    }
    fd_pool[fd].inode_state = USEFUL;
    IDE_READ(fd_pool[fd].inode_idx = fd_pool[fd].inode.next, &fd_pool[fd].inode);
    return 0;
}

int fd_buffer_flush(int fd) {
    if (fd_pool[fd].buffer_state == NEED_WASHING) {
        IDE_WRITE(fd_pool[fd].buffer_idx, &fd_pool[fd].buffer);
    }
    fd_pool[fd].buffer_state = GARBAGE;
    return 0;
}
// predicate: after this method, fd_pool[fd].buffer is ready/dirty
int fd_buffer_fetch(int fd) {
    if (fd_pool[fd].buffer_state == GARBAGE) {
        my_assert(fd_pool[fd].offset % SECTSIZE == 0);
        if (fd_pool[fd].offset % INODE_CAPACITY == 0) { // run out of current inode
            fd_inode_flush(fd);
            fd_pool[fd].inode_state = GARBAGE;
            fd_inode_state_init(fd);
        }
        int index = (int) (fd_pool[fd].offset / SECTSIZE % N_INODE_DATABLOCK);
        if (fd_pool[fd].inode.dataBlocks[index] == 0) {
            fd_pool[fd].inode_state = NEED_WASHING;
            fd_pool[fd].buffer_idx = fd_pool[fd].inode.dataBlocks[index] = find_empty_inode();
        } else IDE_READ(fd_pool[fd].buffer_idx = fd_pool[fd].inode.dataBlocks[index], &fd_pool[fd].buffer);
        fd_pool[fd].buffer_state = USEFUL;
    }
    return 0;
}

int min(int a, int b) {
    return a < b ? a : b;
}

static void internal_open(int fd) {
    fd_pool[fd].buffer_state = fd_pool[fd].inode_state = GARBAGE;
    fd_pool[fd].offset = 0;
    fd_pool[fd].opened = true;
}

int fs_open(const char *pathname, int flags) {
    if (!pathname || !*pathname) return E_EMPTYPATH;
    int file_slot = -1;
    for (int i = 0; i < ENTRY_COUNT; ++i) {
        if (file_slot < 0 && !dir[i].fileName[0]) file_slot = i;
        if (!strncmp(pathname, dir[i].fileName, FILENAME_LENGTH))
            for (int j = 0; j < FDPOOL_SIZE; ++j) if (!fd_pool[j].opened) {
                if (!(fd_pool[j].index = i) && (fd_pool[j].writable = (flags & O_RDWR) != 0)) return E_ACCESS;
                internal_open(j);
                return j;
            }
    }
    if (!(flags & O_CREAT)) return E_NOENT;
    if (file_slot < 0) return E_DQUOT;
    my_assert(strncmp(pathname, "kernel.bin", FILENAME_LENGTH) ? file_slot > 0 : file_slot == 0);
    for (int j = 0; j < FDPOOL_SIZE; ++j) if (!fd_pool[j].opened) {
        fd_pool[j].index = file_slot;
        state_dir = NEED_WASHING;
        if(flags & O_RDWR) fd_pool[j].writable = true;
        internal_open(j);
        dir[file_slot].inodeOffset = (uint32_t) -1; // not allocated
        dir[file_slot].fileSize = 0;
        strncpy(dir[file_slot].fileName, pathname, FILENAME_LENGTH);
        return j;
    }
    return E_NOENT;
}

int fs_read(int fd, void *buf, int len) {
    my_assert(fd_pool[fd].opened);
    len = min(len, (int) (dir[fd_pool[fd].index].fileSize - fd_pool[fd].offset));
    int n_read = 0;
    while (n_read < len) {
        fd_buffer_fetch(fd);
        int offset = (int) (fd_pool[fd].offset % SECTSIZE), count = min(len - n_read, SECTSIZE - offset);
        fd_pool[fd].offset += count;
        memcpy((uint8_t *) buf + n_read, fd_pool[fd].buffer + offset, (size_t) count);
        n_read += count;
        if ((fd_pool[fd].offset % SECTSIZE) == 0) {
            fd_buffer_flush(fd);
        }
    }
    return n_read;
}

int fs_write(int fd, const void *buf, int len) {
    my_assert(fd_pool[fd].opened);
    my_assert(fd_pool[fd].writable);

    int result = 0;
    while (result < len) {
        int offset = (int) (fd_pool[fd].offset % SECTSIZE), count = min(len - result, SECTSIZE - offset);
        fd_buffer_fetch(fd);
        fd_pool[fd].buffer_state = NEED_WASHING;
        fd_pool[fd].offset += count;
        memcpy(fd_pool[fd].buffer + offset, (uint8_t *) buf + result, (size_t) count);
        result += count;
        if (dir[fd_pool[fd].index].fileSize < fd_pool[fd].offset) {
            dir[fd_pool[fd].index].fileSize = (uint32_t) fd_pool[fd].offset;
            state_dir = NEED_WASHING;
        }
        if (offset + count == SECTSIZE) {
            fd_buffer_flush(fd);
        }
    }
    return result;
}

int fs_lseek(int fd, int offset, int whence) {
    my_assert(fd_pool[fd].opened);
    switch (whence) {
        case SEEK_SET:
            fd_buffer_flush(fd);
            fd_inode_flush(fd);
            fd_pool[fd].offset = 0;
            fd_pool[fd].inode_state = GARBAGE;
            return 0;
        case SEEK_CUR:
            fd_pool[fd].offset += offset;   // todo; SIGH
            return 0;
        case SEEK_END:
            fd_pool[fd].offset += dir[fd_pool[fd].index].fileSize + offset; // not even bother to todo
            return 0;

        default: my_assert(0);
    }
}

int fs_close(int fd) {
    my_assert(fd_pool[fd].opened);
    fd_pool[fd].opened = false;
    my_assert(fd_inode_flush(fd) == 0);
    my_assert(fd_buffer_flush(fd) == 0);
    my_assert(fs_flush() == 0);
    return 0;
}
