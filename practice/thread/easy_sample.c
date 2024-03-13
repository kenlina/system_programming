#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

pthread_t ntid;
void printids(const char *s){
    pid_t pid;
    pthread_t tid;

    pid = getpid();
    tid = pthread_self();
    printf("%s pid %u parent %u tid %u (0x%x)\n", s, (unsigned int)pid, (unsigned int)getppid(),(unsigned int)tid, (unsigned int)tid);
}
void *thr_fn(void *arg){
    printids("new thread: ");
    return (void*)0;    
}
int main(){
    int err;
    err = pthread_create(&ntid, NULL, thr_fn, NULL);
    if(err != 0){
        perror("can't create thread");
        return -1;
    }
    printids("main thread:");
    sleep(1);
    
    return 0;
}