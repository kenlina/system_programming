#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
struct foo{
    int a,b,c,d;
};
void printfoo(const char *s, const struct foo *fp){
    printf("%s", s);
    printf("    struct at 0x%x\n", (unsigned)fp);
    printf("    foo.a = %d\n", fp->a);
    printf("    foo.b = %d\n", fp->b);
    printf("    foo.c = %d\n", fp->c);
    printf("    foo.d = %d\n", fp->d);
}
void *thr_fn1(void *arg){
    struct foo foo = {1, 2, 3, 4};
    
    printfoo("thread 1:\n", &foo);
    pthread_exit((void *)&foo);
}
void *thr_fn2(void *arg){
    printf("thread 2: ID is %ld\n", pthread_self());
    pthread_exit((void *)0);
}
int main(){
    pthread_t tid1, tid2;
    struct foo *fp;

    pthread_create(&tid1, NULL, thr_fn1, NULL);
    pthread_join(tid1, (void *)&fp);

    sleep(1);
    printf("parent starting second thread\n");

    pthread_create(&tid2, NULL, thr_fn2, NULL);

    sleep(1);
    printfoo("main thread:\n", fp);
    return 0;
}