#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
int done = 0;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c = PTHREAD_COND_INITIALIZER;

void *spawned(void *arg){

    printf("spawned thread\n");
    sleep(1);

    // cs
    pthread_mutex_lock(&m);
    done = 1;
    pthread_cond_signal(&c);
    pthread_mutex_unlock(&m);
    // cs

    return NULL;
}

int main(){
    pthread_t p;
    pthread_create(&p, NULL, spawned, NULL);
    
    // cs
    pthread_mutex_lock(&m);
    while(done == 0)
        pthread_cond_wait(&c, &m);
    pthread_mutex_unlock(&m);
    // cs

    printf("main thread\n");
    return 0;
}