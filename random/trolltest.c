#include <unistd.h>
#include <fcntl.h>
int main(){
  int fd = open("trollfile", O_RDWR);
  while(1){
    write(fd, "test", 5);
    sleep(1);
  }
}
