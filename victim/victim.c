#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>


char* path  = "test_file";

int main(){

  int fd = open(path, O_RDWR);
  printf("%d\n", fd);
  while(1){

  }

  return 0;
}
