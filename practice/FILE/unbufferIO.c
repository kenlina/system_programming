#include <unistd.h>
#include <stdio.h>

int main(){
	char buf[100];
	ssize_t n;

	while ( (n=read(STDIN_FILENO,buf,100)) != 0 ){
		printf("Read %ld bytes\n", n);			
		write(STDOUT_FILENO,buf,n);
	}
	return 0;
}
