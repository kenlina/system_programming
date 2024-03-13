#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
int main(){
    int p[2];
    char *argv[2]; 

    argv[0] = "wc";
    argv[1] = 0;

    pipe(p);

    if(fork() == 0){
        close(0);
        dup(p[0]);
        close(p[0]);
        close(p[1]);
        execv("/bin/wc", argv);
    }
    else{
        close(p[0]);
        write(p[1], "hello world\n", 13);
        close(p[1]);
    }
    return 0;
}