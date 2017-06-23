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

const char filename_kernel[] = "kernel.bin";

struct {
    int index;
    size_t offset;
    uint8_t buffer[SECTSIZE];
    INode inode;
    uint32_t buffer_idx, inode_idx;
    bool opened, writable;
    uint32_t buffer_state, inode_state;
} fdPool[FDPOOL_SIZE];

void fs_init() {
    ide_read(FSOFFSET_BITMAP, &bitmap, FSLENGTH_BITMAP);
    ide_read(FSOFFSET_DIR, &dir, 1);
    state_bitmap = state_dir = USEFUL;
}

int fs_flush() {
    if (state_bitmap == NEED_WASHING) ide_write(FSOFFSET_BITMAP, &bitmap, FSLENGTH_BITMAP);
    if (state_dir == NEED_WASHING) ide_write(FSOFFSET_DIR, &dir, 1);
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
        bitmap[i] |= 1 << bit_count;
        state_bitmap = NEED_WASHING;
        return (i << 5) + bit_count;
    }
    panic(0, "No more empty inode available");
}

int fd_inode_flush(int fd) {
    if (fdPool[fd].inode_state == NEED_WASHING) {
        IDE_WRITE(fdPool[fd].inode_idx, &fdPool[fd].inode);
        fdPool[fd].inode_state = USEFUL;
    }
    return 0;
}

void fd_set_inode(int fd) {
    fdPool[fd].inode_idx = dir[fdPool[fd].index].inodeOffset = (uint32_t) find_empty_inode();
    state_dir = NEED_WASHING;
    memset(&fdPool[fd].inode, 0, SECTSIZE);
    fdPool[fd].inode_state = NEED_WASHING;
}

int fd_alloc_next_inode(int fd) {
    fdPool[fd].inode.next = (uint32_t) find_empty_inode();
    fdPool[fd].inode_state = NEED_WASHING;
    fd_inode_flush(fd);
    fdPool[fd].inode_idx = fdPool[fd].inode.next;
    memset(&fdPool[fd].inode, 0, SECTSIZE);
    fdPool[fd].inode_state = NEED_WASHING;
    return 0;
}

int fd_inode_state_init(int fd) {
    my_assert(fdPool[fd].offset % INODE_CAPACITY == 0);
//    my_assert(fdPool[fd].inode_state == GARBAGE);

    if (fdPool[fd].offset == 0) {
        if (dir[fdPool[fd].index].inodeOffset == -1) fd_set_inode(fd);
        else {
            IDE_READ(fdPool[fd].inode_idx = dir[fdPool[fd].index].inodeOffset, &fdPool[fd].inode);
            fdPool[fd].inode_state = USEFUL;
        }
        return 0;
    }
    // already started reading
    if (fdPool[fd].inode.next == 0) fd_alloc_next_inode(fd);
    else {
        IDE_READ(fdPool[fd].inode_idx = fdPool[fd].inode.next, &fdPool[fd].inode);
        fdPool[fd].inode_state = USEFUL;
    }
    return 0;
}

int fd_buffer_flush(int fd) {
    if (fdPool[fd].buffer_state == NEED_WASHING) {
        IDE_WRITE(fdPool[fd].buffer_idx, &fdPool[fd].buffer);
    }
    fdPool[fd].buffer_state = GARBAGE;
    return 0;
}
// predicate: after this method, fdPool[fd].buffer is ready/dirty
int fd_buffer_fetch(int fd) {
    if (fdPool[fd].buffer_state == GARBAGE) {
        my_assert(fdPool[fd].offset % SECTSIZE == 0);
        if (fdPool[fd].offset % INODE_CAPACITY == 0) { // run out of current inode
            fd_inode_flush(fd);
            fdPool[fd].inode_state = GARBAGE;
            fd_inode_state_init(fd);
        }
        int index = (int) (fdPool[fd].offset / SECTSIZE % N_INODE_DATABLOCK);
        if (fdPool[fd].inode.dataBlocks[index] == 0) {
            fdPool[fd].buffer_idx = fdPool[fd].inode.dataBlocks[index] = find_empty_inode();
            fdPool[fd].inode_state = NEED_WASHING;
        } else IDE_READ(fdPool[fd].buffer_idx = fdPool[fd].inode.dataBlocks[index], &fdPool[fd].buffer);
        fdPool[fd].buffer_state = USEFUL;
    }
    return 0;
}

int min(int a, int b) {
    return a < b ? a : b;
}

static void internal_open(int fd) {
    fdPool[fd].opened = true;
    fdPool[fd].offset = 0;
    fdPool[fd].buffer_state = fdPool[fd].inode_state = GARBAGE;
}

int fs_open(const char *pathname, int flags) {
    if (!pathname || !*pathname) return E_EMPTYPATH;
    int file_slot = -1;
    for (int i = 0; i < ENTRY_COUNT; ++i) {
        if (file_slot < 0 && !dir[i].fileName[0]) file_slot = i;
        if (!strncmp(pathname, dir[i].fileName, FILENAME_LENGTH))
            for (int j = 0; j < FDPOOL_SIZE; ++j) if (!fdPool[j].opened) {
                if (!(fdPool[j].index = i) && (fdPool[j].writable = (flags & O_RDWR) != 0)) return E_ACCESS;
                internal_open(j);
                return j;
            }
    }
    if (flags & O_CREAT) {
        if (file_slot < 0) return E_DQUOT;
        my_assert(strncmp(pathname, "kernel.bin", FILENAME_LENGTH) ? file_slot > 0 : file_slot == 0);
        for (int j = 0; j < FDPOOL_SIZE; ++j) if (!fdPool[j].opened) {
            fdPool[j].index = file_slot;
            if(flags & O_RDWR) fdPool[j].writable = true;
            internal_open(j);
            strncpy(dir[file_slot].fileName, pathname, FILENAME_LENGTH);
            dir[file_slot].fileSize = 0;
            dir[file_slot].inodeOffset = (uint32_t) -1; // not allocated
            state_dir = NEED_WASHING;
            return j;
        }
    }
    return E_NOENT;
}

int fs_read(int fd, void *buf, int len) {
    my_assert(fdPool[fd].opened);
    len = min(len, (int) (dir[fdPool[fd].index].fileSize - fdPool[fd].offset));
    int n_read = 0;
    while (n_read < len) {
        fd_buffer_fetch(fd);
        int offset = (int) (fdPool[fd].offset % SECTSIZE), count = min(len - n_read, SECTSIZE - offset);
        memcpy((uint8_t *) buf + n_read, fdPool[fd].buffer + offset, (size_t) count);
        n_read += count;
        fdPool[fd].offset += count;
        if ((fdPool[fd].offset % SECTSIZE) == 0) {
            fd_buffer_flush(fd);
        }
    }
    return n_read;
}

int fs_write(int fd, const void *buf, int len) {
    my_assert (fdPool[fd].opened);
    my_assert (fdPool[fd].writable);

    int result = 0;
    while (result < len) {
        fd_buffer_fetch(fd);
        int offset = (int) (fdPool[fd].offset % SECTSIZE), count = min(len - result, SECTSIZE - offset);
        memcpy(fdPool[fd].buffer + offset, (uint8_t *) buf + result, (size_t) count);
        fdPool[fd].buffer_state = NEED_WASHING;
        result += count;
        fdPool[fd].offset += count;
        if (dir[fdPool[fd].index].fileSize < fdPool[fd].offset) {
            dir[fdPool[fd].index].fileSize = (uint32_t) fdPool[fd].offset;
            state_dir = NEED_WASHING;
        }
        if (offset + count == SECTSIZE) {
            fd_buffer_flush(fd);
        }
    }
    return result;
}

int fs_lseek(int fd, int offset, int whence) {
    my_assert(fdPool[fd].opened);
    switch (whence) {
        case SEEK_SET:
            fd_buffer_flush(fd);
            fd_inode_flush(fd);
            fdPool[fd].offset = 0;
            fdPool[fd].inode_state = GARBAGE;
            return 0;
        case SEEK_CUR:
            fdPool[fd].offset += offset;
            return 0;
        case SEEK_END:
            fdPool[fd].offset += dir[fdPool[fd].index].fileSize + offset;
            return 0;

        default: my_assert(0);
    }
}

int fs_close(int fd) {
    my_assert(fdPool[fd].opened);
    fdPool[fd].opened = false;
    my_assert(fd_buffer_flush(fd) == 0);
    my_assert(fd_inode_flush(fd) == 0);
    my_assert(fs_flush() == 0);
    return 0;
}
