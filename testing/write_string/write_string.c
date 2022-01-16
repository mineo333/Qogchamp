#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char** argv){
    int fd = open("tfile", O_CREAT | O_WRONLY | O_APPEND, 0700);
    
    for(int i = 0; i<argc; i++){
      write(fd, *(argv+i), strlen(*(argv+i)));
    }
}