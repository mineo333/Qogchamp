#include <unistd.h>
#include <fcntl.h>

int main(){
    char* str = "test this write";
    int fd = open("test_file", O_RDWR | O_APPEND);
    write(fd, str, 16);
    return 0;
}