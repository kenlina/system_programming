#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <stdlib.h>
#include <string.h>

int main(){
    int i;
    struct timeval timeout;
    fd_set  master_set, working_set;
    char buf[1024];

    FD_ZERO(&master_set);
    FD_SET(0, &master_set);
    /*  寫在這邊的話它指初始化第一次timeout 
        之後被select改過後timeout都會是0
        所以應該要放在while迴圈裡面重置它才會達到每次都等5秒
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    */
    i = 0;

    while(1){
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        memcpy( &working_set, &master_set, sizeof(master_set));
        select(1, &working_set, NULL, NULL, &timeout); // 每次select時間到之後它就會把timeout扣掉過去的時間
        //printf("timeout.tv_sec: %ld\n", timeout.tv_sec);
        if( FD_ISSET(0, &working_set) ){
            fgets(buf, sizeof(buf), stdin);
            fputs(buf, stdout);
        }
        printf("iteration: %d\n", i++ ); 
    }
    
    return 0;
}