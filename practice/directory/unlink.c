#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
int main(){
    if ( open("tempfile", O_RDWR) < 0 ){
        printf("open failed\n");
        return -1;
    }
    if (unlink("tempfile") < 0){
        printf("unlink error\n");
        return -1;
    }
    printf("file unlinked\n");
    sleep(15);
    printf("done\n");
    exit(0);
}