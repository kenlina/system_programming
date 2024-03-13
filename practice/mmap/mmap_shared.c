#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <wait.h>
#include <string.h>
typedef struct {
    pthread_mutex_t lock;
    char buf[BUFSIZ];
} buftype;

int main(){
    int fd;
    buftype *bufptr;
    pthread_mutexattr_t attr;

    fd = open("/dev/zero", O_RDWR);
    bufptr = (buftype*)mmap(NULL, sizeof(buftype), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(bufptr == MAP_FAILED){
        perror("mmap");
        return 0;
    }
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&bufptr->lock, &attr);

    if(fork() == 0){
        pthread_mutex_lock(&bufptr->lock);
        // cs

        strcat(bufptr->buf, "i am child\n");
        printf("%s", bufptr->buf);

        //cs
        pthread_mutex_unlock(&bufptr->lock);



        // close resource
        munmap(bufptr, sizeof(buftype));
        pthread_mutexattr_destroy(&attr);
        close(fd);
    }
    else{
        pthread_mutex_lock(&bufptr->lock);
        //cs

        strcat(bufptr->buf, "i am parent\n");
        printf("%s", bufptr->buf);

        //cs
        pthread_mutex_unlock(&bufptr->lock);



        // close resource
        wait(NULL);
        munmap(bufptr, sizeof(buftype));
        pthread_mutexattr_destroy(&attr);
        close(fd);
    }
    return 0;

}