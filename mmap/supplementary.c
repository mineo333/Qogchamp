#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
int main(){
  int fd = open("./testfile", O_RDONLY);
  while(1){
    char* buf = (char*)malloc(10);
    read(fd, buf, 10);
    free(buf);
  }





  return 0;
}
