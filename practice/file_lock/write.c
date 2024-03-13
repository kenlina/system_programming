#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
int main(){
    int fd = openat(AT_FDCWD, "file", O_RDWR, 0644);
    if(fd < 0) return -1;
    if(write(fd, "hello\n", 6) < 0){
        printf("write error\n");
        return -1;
    }
    printf("write successful\n");
    return 0;
}