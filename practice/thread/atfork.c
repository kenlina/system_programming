#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;

void prepare(){
    printf("preparing locks...\n");
    pthread_mutex_lock(&lock1);
    pthread_mutex_lock(&lock2);
}
void parent(){
    printf("parent unlocking locks...\n");
    pthread_mutex_unlock(&lock1);
    pthread_mutex_unlock(&lock2);
}
void child(){
    printf("child unlocking locks...\n");
    pthread_mutex_unlock(&lock1);
    pthread_mutex_unlock(&lock2);
}
void *thr_fn(void* arg){
    printf("thread started...\n");
    pause();
    return 0;
}

int main(){
    pthread_t tid;

#if defined(BSD) || defined(MACOS)
    printf("pthread_atfork is unsupported\n");
#else 
    pthread_atfork(prepare, parent, child);
    pthread_create(&tid, NULL, thr_fn, NULL);
    sleep(2);
    printf("parent about to fork\n");
    if(fork() == 0){
        printf("child returned from fork\n");
    }
    else{
        printf("parent returned from fork\n");
    }
#endif 

    return 0;

}