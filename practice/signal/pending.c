#include <stdio.h>
#include <unistd.h>
#include <signal.h>
void sig_quit(int signo){
    printf("caught SIGQUIT\n");
    if(signal(SIGQUIT, SIG_DFL) == SIG_ERR){
        printf("can't reset SIGQUIT\n");
        return;
    }
}

int main(){
    sigset_t new, old, pend;
    if(signal(SIGQUIT, sig_quit) == SIG_ERR){
        printf("can't catch SIGQUIT\n");
        return -1;
    }
    sigemptyset(&new);
    sigaddset(&new, SIGQUIT);
    if(sigprocmask(SIG_BLOCK, &new, &old) < 0){
        printf("SIG_BLOCK error\n");
        return -1;
    }

    sleep(5);

    if(sigpending(&pend) < 0){
        printf("sigpending error\n");
        return -1;
    }
    if(sigismember(&pend, SIGQUIT)){
        printf("\nSIGQUIT pending\n");
    }
    if(sigprocmask(SIG_SETMASK, &old, NULL) < 0){
        printf("SIG_SETMASK error\n");
        return -1;
    }

    printf("SIGQUIT unblocked\n");

    sleep(5);
    return 0;
}