#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
int main(){
    int n;
    int fd[2];
    pid_t pid;
    char line[100];

    if(pipe(fd) < 0){
        printf("pipe error\n");
        exit(1);
    }
    printf("fd[0] = %d\nfd[1] = %d\n",fd[0],fd[1]);

    if((pid=fork())<0){
        printf("fork error\n");
        exit(1);
    } else if (pid >0){ /*Parent process*/
        close(fd[0]);
        write(fd[1],"hello world\n", 12);
    } else{ /*Child process*/
        close(fd[1]);
        n = read(fd[0], line, 100);
        write(1, line, n);
    }
    exit(0);
}