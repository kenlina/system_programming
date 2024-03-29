#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
void sig_usr(int);

int main(){
    if( signal(SIGUSR1, sig_usr) == SIG_ERR ){
        printf("can't catch SIGUSR1\n");
        return -1;
    }
    if( signal(SIGUSR2, sig_usr) == SIG_ERR ){
        printf("can't catch SIGUSR2\n");
        return -1;
    }
    for(;;)
        pause();
}

void sig_usr(int signo){
    if( signo == SIGUSR1 )
        printf("received SIGUSR1\n");
    else if( signo == SIGUSR2 )
        printf("received SIGUSR2\n");
    else
        printf("received signal %d\n", signo);
}