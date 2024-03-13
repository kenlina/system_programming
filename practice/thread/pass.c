#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
void *PrintNo(void *ptr){
    printf("My number is %d\n", *((int *)ptr));
    pthread_exit(NULL);
}
int main(){
    pthread_t tid[10];
    int no;
    for ( no = 0; no < 10; ++no){
        pthread_create(&tid[no], NULL, PrintNo, (void *)&no);
    }
    pthread_exit(NULL);
}