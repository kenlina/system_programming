#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
int main(){
    pid_t pid;
    if((pid=fork())<0){
        printf("fork error\n");
        exit(0);
    } else if(pid > 0) exit(0);

    sleep(10);
    printf("I'm child process");
    exit(0);
}