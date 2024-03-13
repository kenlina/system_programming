#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
int main( int argc, char *argv[] ) // merger.c
{
    int i, pid;
    int status;
    int fd[argc][2]; //這裡很髒
    char buf[argc][BUFSIZ];
    int pids[argc];

    for (i=1; i<argc; i++)
    {
        if(pipe(fd[i]) < 0){
            printf("pipe error\n");
            exit(1);
        }
        pid = fork();
        if ( pid == 0 )
        {
            close(fd[i][0]);
            dup2(fd[i][1], 1);
            execlp( argv[i], argv[i], (char *) 0 );
        }
        pids[i] = pid;
        close(fd[i][1]);
        
    }
    // 收集輸出
    for (i=1; i<argc; i++){
        if((pid=wait(&status))<0){
            printf("wait error\n");
            exit(1);
        }
        for(int j = 1; j < argc; ++j){
            if(pids[j]==pid)
                read(fd[j][0], buf[j], BUFSIZ);
        }
    }


    printf("all process done\n");
    // 開始輸出結果
    for (i=1; i<argc; i++){
        printf("argv[%d] result =\n%s", i, buf[i]);
    }
    exit(0);
 }