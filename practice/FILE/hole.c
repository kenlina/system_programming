#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
char buf1[]="abcdefghij";
char buf2[]="ABCDEFGHIJ";

int main(){
	int fd;

	fd = open("file.hole",O_RDWR | O_CREAT,700);
	
	if(write(fd, buf1, 10) != 10)
		printf("buf1 write error\n");
	if(lseek(fd, 16384, SEEK_SET) == -1)
		printf("lseek error\n");
	if(write(fd,buf2,10) != 10)
		printf("buf2 write error\n");
	return 0;	
 	
}
