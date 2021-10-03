#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
int main(){
	int fd  = open("test_file", O_RDONLY | O_CREAT);
	char* buf = malloc(10);
	read(fd, buf, 10);
	printf("%s\n", buf);
	return 0;
}
