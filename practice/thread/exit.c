#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
void *thr_fn1(void *arg){
    printf("thread1 returning\n");
    return (void *)1;
}
void *thr_fn2(void *arg){
    printf("thread2 exiting\n");
    pthread_exit((void *)2);
}

int main(){
    pthread_t tid1, tid2;
    void* tret;
    
    printf("(void *) size >= (int) size 才可以用型態轉換傳int參數到thread參數裡面\n");
    printf("(void *) size: %ld\n(int)    size: %ld\n", sizeof(void *), sizeof(int));


    pthread_create(&tid1, NULL, thr_fn1, NULL);
    pthread_create(&tid2, NULL, thr_fn2, NULL);
    
    pthread_join(tid1, &tret);
    printf("thread 1 exit code %d\n", (int)tret );

    pthread_join(tid2, &tret);
    printf("thread 2 exit code %d\n", (int)tret );

    exit(0);
}