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
    /*if(*path == 'Q'){
      break;
    }*/

    //fd = open(path, O_WRONLY);
    //write(fd,path,9);
    //close(fd);
    write(1,path,8);


  }
  //close(fd);

  return 0;
}
