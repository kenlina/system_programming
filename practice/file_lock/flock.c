#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <poll.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>

int main(){
    int fd = openat(AT_FDCWD, "file", O_RDWR, 0644);
    if(fd < 0) return -1;
    struct flock lock = {
        .l_type = F_WRLCK,     // 設置為寫鎖
        .l_whence = SEEK_SET,  // 偏移量是相對於文件開頭
        .l_start = 25,          // 從文件開頭開始
        .l_len = 25             // 鎖定到文件末尾
        // l_pid 通常由系統設置，不需要手動初始化
    };
    if(fcntl(fd, F_SETLK, &lock) == -1){
        printf("set lock error\n");
    }
    else{
        printf("set lock success\n");
    }
    if(fcntl(fd, F_GETLK, &lock) == -1){
        printf("get lock error\n");
    }
    if(lock.l_type == F_UNLCK){
        printf("文件未被上鎖\n");
    }
    else{
        printf("锁的类型: %s\n", (lock.l_type == F_RDLCK) ? "读锁" : "写锁");
        printf("锁定的起始位置: %ld\n", lock.l_start);
        printf("锁定的长度: %ld\n", lock.l_len);
        printf("持有锁的进程ID: %d\n", lock.l_pid);    
    }
    while (1){}
    
    return 0;
}