#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
char *env_init[] = {"USER=unknown", "PATH=/tmp", NULL};
int main(){
    pid_t pid;
    if((pid=fork())<0){
        printf("fork error\n");
        exit(1);
    }
    else if (pid == 0){
        if(execle("./echoall", "echoall", "myarg1", "myarg2", (char *)0, env_init) < 0){
            printf("execle error\n");
            exit(1);
        }
    }

    if(waitpid(pid, NULL, 0)<0){
        printf("waitpid error\n");
        exit(1);
    }

    if((pid=fork())<0){
        printf("fork error\n");
        exit(1);
    }
    else if (pid == 0){
        if(execlp("./echoall", "echoall", "only 1 arg", (char *)0) < 0){
            printf("execlp error\n");
            exit(1);
        }
    }
    exit(0);
}