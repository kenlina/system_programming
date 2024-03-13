#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
void pr_mask(char *str){
    sigset_t sigset, mask;
    int errno_save = errno;
    
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGINT);
    sigaddset(&sigset, SIGUSR1);

    // 設置sigmask
    if( sigprocmask(SIG_BLOCK, &sigset, NULL) < 0)
        return;
    // 得到當前sigmask
    if( sigprocmask(0, NULL, &mask) < 0)
        return;


    printf("%s", str);
    if(sigismember(&mask, SIGINT)) printf("SIGINT ");
    if(sigismember(&mask, SIGQUIT)) printf("SIGQUIT ");
    if(sigismember(&mask, SIGUSR1)) printf("SIGUSR1 ");
    if(sigismember(&mask, SIGALRM)) printf("SIGALRM ");

    printf("\n");

    errno = errno_save;
}

int main(){
    pr_mask("mask: ");
    return 0;
}

