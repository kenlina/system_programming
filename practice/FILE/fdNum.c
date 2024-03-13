#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

int main(){
	int fd1,fd2;
	fd1 = open("unbufferIO.c",O_RDONLY,0);
	close(fd1);
	fd2 = open("createFILE.c",O_RDONLY,0);
	printf("fd2 = %d\n", fd2);
	exit(0);
}
