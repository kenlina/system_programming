#include <unistd.h>
#include <setjmp.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

static sigjmp_buf jmp;
static volatile sig_atomic_t canjmp;

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
void sig_usr1(int signo){
    time_t starttime;

    if( canjmp == 0) return;
    
    pr_mask("starting sig_usr1: ");
    alarm(3);
    starttime = time(NULL);
    for(;;){
        if( time(NULL) > starttime + 5 ) break;
    }
    pr_mask("finishing sig_usr1: ");

    canjmp = 0;
    siglongjmp(jmp, 1);
}

void sig_alrm(int signo){
    pr_mask("\nin sig_alrm: ");
}

int main(){
    signal(SIGUSR1, sig_usr1);
    signal(SIGALRM, sig_alrm);
    pr_mask("starting main: ");

    if(sigsetjmp(jmp, 1)){
        pr_mask("ending main: ");
        return 0;
    }
    canjmp = 1;

    for(;;) pause();
}