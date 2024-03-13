#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
int main(){
    pid_t pid;

    if((pid=fork())<0){
        printf("fork error\n");
        exit(1);
    }else if(pid==0){
        if((pid=fork())<0){
            printf("fork error\n");
            exit(1);
        } else if (pid > 0){
            //printf("first child pid = %d\n",getpid());
            exit(0);
        } 
        
        sleep(2);
        printf("second child, parent pid = %d\n", getppid());
        exit(0);
    }

    if(waitpid(pid,NULL,0)!=pid){
        printf("waitpid error\n");
        exit(1);
    }

    exit(0);
}