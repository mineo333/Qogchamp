#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

int fd_stdin, fd_stdout, fd_stderr;

int main(int argc, char** argv){


   
    FILE* fd = fopen("tfile", "a");
    int pt = open("/dev/ptmx", O_RDWR | O_CLOEXEC);
    printf("%d\n", pt);
    fprintf(fd, "pt fd: %d\n", pt);


    char* buf = malloc(1024);


    int ret = read(pt, buf, 1024);
    if(ret < 0)
      fprintf(fd,"errno: %d\n", errno);
    else
      fprintf(fd,"string:%s, size: %d\n", buf, ret);
    //for(int i = 0; i<argc; i++){
      //write(fd, *(argv+i), strlen(*(argv+i)));
    //}
}