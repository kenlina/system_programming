#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
int main(){
    char *argv[4];
    
    argv[0] = "echo";
    argv[1] = "hello";
    argv[2] = "gg";
    argv[3] = NULL;
    execv("/usr/bin/echo", argv);
    printf("execl error\n");
    return -1;
}