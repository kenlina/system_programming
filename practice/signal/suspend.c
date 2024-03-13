#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>

void pr_mask(char *str){
    sigset_t mask;
    int errno_save = errno;

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
void sig_int(int signo){
    pr_mask("\nin sig_int: ");
}
int main(){
    sigset_t new, old, wait;
    pr_mask("program start: ");
    signal(SIGINT, sig_int);
    sigemptyset(&wait), sigemptyset(&new);
    sigaddset(&wait, SIGUSR1);
    sigaddset(&new, SIGINT);

    sigprocmask(SIG_BLOCK, &new, &old);
    /*
        critical region

    
    
    */
    pr_mask("in critical region: ");
    sleep(5);
    // pause
    sigsuspend(&wait);
    pr_mask("after return from sigsuspend: ");

    // 還原
    sigprocmask(SIG_SETMASK, &old, NULL);
    pr_mask("program exit: ");
    return 0;
}