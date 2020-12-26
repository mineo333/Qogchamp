#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
const char* path = "/etc/passwd";
int main(){
  int fd;

  while(1){
    fd = open("passwd", O_RDONLY);

    //  printf("Path: %p\n", (void*)path);


    close(fd);

  }

  return 0;
}
