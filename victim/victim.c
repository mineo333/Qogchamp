#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>


char* path  = "cockinmyballs";

int main(){

  int fd = open(path, O_RDWR);
  while(1){

  }

  return 0;
}

/*while(1){
  int fd = open(path, O_RDWR);
  if(*path == 'Q'){
    return 0;

  }
  close(fd);


}*/
