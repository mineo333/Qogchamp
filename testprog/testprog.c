#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

int main(){
  int fd;
  char* path = "/etc/passwd";
  while(1){
    fd = open(path, O_RDONLY);

      printf("%s",path);


    close(fd);

  }

  return 0;
}
