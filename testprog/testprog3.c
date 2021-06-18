#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>


char* path   = "/dev/null";

int main(){

  while(1){
    int fd = open(path, O_RDWR);
    if(*path == 'Q'){
      return 0;

    }
    //mprotect(path,10,PROT_READ|PROT_WRITE);
    //*path = 'A';
    //printf("%s\n", path);
    //fd = open(path, O_WRONLY);
    //write(fd,path,9);
    //close(fd);
    close(fd);


  }


  return 0;
}
