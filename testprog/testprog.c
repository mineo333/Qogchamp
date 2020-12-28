#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

int main(){
  int fd;
  char* path = "POGCHAMP";
  while(1){
    fd = open("/etc/passwd", O_RDONLY);

      printf("Path: %s\n", path);


    close(fd);

  }

  return 0;
}
