#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
int main(){
  int fd;
//  int devnull;
  char* addr;
  fd = open("./testfile", O_RDWR);
  addr = mmap((void*)0x7f751c27a000, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0); //make the mmap cover a page
  printf("%lx\n", (unsigned long)addr);
  //*addr = 'Q'; // this operation makes the page copy on write

  printf("%s\n", addr); // we need to print it out because of demand paging. The page won't exist unless we access the contents
  while(1){

  }
  munmap(addr, 4);
  close(fd);
  return 0;
}
