#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>
#include<fcntl.h>
#include<sys/types.h>
int main(int argc, char *argv[]){
	int fd1, fd2;
	fd1 = open ("infile",O_RDONLY );
	fd2 = open("outfile", O_WRONLY | O_CREAT | O_TRUNC, 0666); 
	//O_TRUNC: 若開啟的檔案存在且是一般檔案，開啟後就將其截短成長度為 0
	

	//code
	dup2(fd1, 0);
	dup2(1, 2);
	dup2(fd2, 1);
	//end
	
	execlp("./a.out", "./a.out", NULL);
	return 0;

}

