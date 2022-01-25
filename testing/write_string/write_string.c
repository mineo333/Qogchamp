#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

int fd_stdin, fd_stdout, fd_stderr;
char* qogchamp = "qogchamp";
int main(int argc, char** argv){


   
    FILE* fd = fopen("tfile", "a");
  //  int pt = open("/dev/qogchamp", O_WRONLY);
    
    fprintf(fd,"made it into the function\n");
    int err = write(1, qogchamp, strlen(qogchamp));
    if(err < 0){
      fprintf(fd,"errno: %d\n", errno);
    }
    fprintf(fd,"made it past the write\n");

    /*char* buf = malloc(1024);


    int ret = read(pt, buf, 1024);
    if(ret < 0)
      fprintf(fd,"errno: %d\n", errno);
    else
      
    //for(int i = 0; i<argc; i++){
      //write(fd, *(argv+i), strlen(*(argv+i)));
    //}*/
}