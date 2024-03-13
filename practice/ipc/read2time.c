#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
int main(){
    pid_t pid;
    int pwr_cre[2], pre_cwr[2];
    char buf[BUFSIZ];
    int byte;
    if(pipe(pwr_cre) < 0 || pipe(pre_cwr) < 0){
        perror("pipe");
        return -1;
    }
    if( (pid=fork()) == 0){     /* child */
        close(pwr_cre[1]);
        if((byte=read(pwr_cre[0], buf, BUFSIZ))<0){
            printf("read error:%d\n", errno);
        }
        printf("child read byte: %d\n",byte);
    }
    else if ( pid > 0) {        /* parent */
        sleep(3);
    }
    return 0;
}