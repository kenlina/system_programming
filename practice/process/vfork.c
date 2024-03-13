#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

int glob = 6;
char buf[] = "a write to stdout\n";

int main(){
    int var;
    pid_t pid;
    var = 88;
   
    printf("before vfork\n");

    if ( (pid = vfork()) < 0){
        printf("vfork error\n");
        exit(0);
    }else if (pid == 0){
        ++glob;
        ++var;
        _exit(0);
    } 
    /*
     * Parent continues here
     */
    printf("pid = %d, glob = %d, var =%d\n", getpid(), glob, var);
    exit(0);
}