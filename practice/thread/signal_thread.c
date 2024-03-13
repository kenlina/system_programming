#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
pthread_t tid;

void *signal_fn(void *arg){
    sigset_t catch;
    int caught;
    sigemptyset(&catch);
    sigaddset(&catch, SIGUSR1);

    for(;;){
        sigwait(&catch, &caught);
        /* 處理收到sigusr1 */
        printf("received signal number: %d\n", caught);
    }
    return NULL;
}

int main(){
    sigset_t block;

    sigemptyset(&block);
    sigaddset(&block, SIGUSR1);
    // 在創新thread之前就要先block了 讓新的有保護力
    pthread_sigmask(SIG_BLOCK, &block, NULL);

    pthread_create(&tid, NULL, signal_fn, NULL);

    for(;;){}

}