#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
// #include <string.h>
// #include <stdlib.h>
int main(){
    int fd[2];
    pipe(fd);
    int psize = 0, w;
    fcntl(fd[1], F_SETFL, O_NONBLOCK);
    for(int i = 0; i < 65537; ++i){
        if((w=write(fd[1], "1", 1))<0){
            perror("write wrong");
            printf("pipe size : %d\n", psize);
            break;
            // printf("Oh dear, something went wrong with read()! %s\n", strerror(errno));
        }
        psize += w;
    }
    close(fd[0]);
    close(fd[1]);
    return 0;

}