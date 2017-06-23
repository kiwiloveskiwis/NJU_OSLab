#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "kernel/fs.h"

int imageFd;

int main(int argc, char * const *argv) {
  assert(argc == 3);
  imageFd = open(argv[1], O_RDWR);
  assert((imageFd >= 0) && "open failed");

  fs_init();
  int fd = fs_open(argv[2], O_RDWR | O_CREAT);
  assert((fd >= 0) && "fs_open failed\n");

  uint8_t buffer[SECTSIZE];
  ssize_t result;
  while ((result = read(STDIN_FILENO, buffer, SECTSIZE)) > 0) {
    int r = fs_write(fd, buffer, (int) result);
    assert((r >= 0) && (r == result) && "fs_write failed\n");
  }
  assert((result >= 0) && "read failed");

  fs_close(fd);
  close(imageFd);
  return 0;
}
