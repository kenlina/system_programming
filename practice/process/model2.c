#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
int main( int argc, char *argv[] ) // merger.c
{
    int i, pid;
    int status;
    int fd[2]; 
    char buf[BUFSIZ];
    int pids[argc];

    if(pipe(fd) < 0){
        printf("pipe error\n");
        exit(1);
    }
    
    for (i=1; i<argc; i++)
    {
        pid = fork();
        if ( pid == 0 )
        {
            close(fd[0]);
            dup2(fd[1], 1);
            execlp( argv[i], argv[i], (char *) 0 );
        }
        pids[i] = pid;
    }
    close(fd[1]);
    
    // 收集輸出
    while (wait(&status)>=0){}
    read(fd[0], buf, BUFSIZ);
    printf("%s", buf); 
    exit(0);
 }