#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
void set_fl(int fd, int flags){
    int val;
    if((val = fcntl(fd,F_GETFL, 0)) < 0)
        fprintf(stderr,"fcntl F_GETFL error\n");

    val |= flags;

    if(fcntl(fd,F_SETFL, val) < 0)
        fprintf(stderr,"fcntl F_SETFL error\n");
}
void clr_fl(int fd, int flags){
    int val;
    if((val = fcntl(fd,F_GETFL, 0)) < 0)
        fprintf(stderr,"fcntl F_GETFL error\n");

    val &= ~flags;

    if(fcntl(fd,F_SETFL, val) < 0)
        fprintf(stderr,"fcntl F_SETFL error\n");
}
char buf[500000];
int main(){
    int ntowrite, nwrite;
    char* ptr;

    ntowrite = read(STDIN_FILENO, buf, sizeof(buf));
    fprintf(stderr,"read %d bytes\n", ntowrite);

    set_fl(STDOUT_FILENO, O_NONBLOCK);

    ptr = buf;

    while(ntowrite > 0){
        errno = 0;
        nwrite = write(STDOUT_FILENO, ptr, ntowrite);
        fprintf(stderr,"nwrite = %d , errno = %d\n", nwrite, errno);

        if(nwrite > 0){
            ptr += nwrite;
            ntowrite -= nwrite;
        }
    }
    clr_fl(STDOUT_FILENO, O_NONBLOCK);

    return 0;

}