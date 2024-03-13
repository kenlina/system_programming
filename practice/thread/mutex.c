#include <stdio.h>
#include <pthread.h>
#define num 4

int share = 0;
pthread_mutex_t mutex;

void *increment_with_lock(void *arg){
    for(int i = 0; i < 100000; ++i){
        pthread_mutex_lock(&mutex);
        ++share;
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

void *increment(void *arg){
    int id = (int)arg;
    for(int i = 0; i < 50; ++i){
        // pthread_mutex_lock(&mutex);
        
        ++share;
        printf("Thread %d: share incremented to %d\n", id, share);
        // pthread_mutex_unlock(&mutex);
    }
    
    pthread_exit(NULL);
}

int main(){
    pthread_t tid[num];
    pthread_mutex_init(&mutex, NULL);
    for (int i = 0; i < num; i++){
        pthread_create(&tid[i], NULL, increment, (void *)i);
    }
    for (int i = 0; i < num; i++){
        pthread_join(tid[i], NULL);
    }
    pthread_mutex_destroy(&mutex);
    printf("share: %d\n", share);

    return 0;
}