#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "kernel/fs.h"


int imageFd;

int main(int argc, char * const *argv) {
  assert((argc >= 3) && "Too few arguments");
  assert( ((imageFd = open(argv[1], O_RDONLY)) >= 0) && "open failed");
  fs_init();
  int fd = fs_open(argv[2], O_RDONLY);
  assert((fd >= 0) && "fs_open failed\n");

  uint8_t buffer[SECTSIZE];
  int result;
  while ((result = fs_read(fd, buffer, SECTSIZE)) > 0) {
    ssize_t r = write(STDOUT_FILENO, buffer, (size_t) result);
    assert((r >= 0) && (r == result) && "write failed");
  }
  assert((result >= 0) && "fs_read failed\n");
  fs_close(fd);

  close(imageFd);
  return 0;
}
