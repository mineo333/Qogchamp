#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
int main(){
  int fd;
  char* addr;
  fd = open("test_file", O_RDWR);
  addr = mmap((void*)0x7f751c27a000, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); //make the mmap cover a page
  printf("%lx\n", (unsigned long)addr);
  msync(addr, 132, MS_SYNC);
  munmap(addr, 4);
  close(fd);
  return 0;
}
