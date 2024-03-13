#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>

int main() {
    clock_t start_time, end_time;
    double elapsed_time;
    int fd = open("file", O_RDWR);
    int in = open("/dev/zero", O_RDONLY);
    // 记录开始时间
    char buf[10];
    int ct = 0;
    start_time = clock();
    while(read(in, buf , sizeof(buf) ) > 0){
        if(write(fd, buf, sizeof(buf)) < 0){
            printf("write error\n");
            break;
        }
        // if (fdatasync(fd) == -1) {
        //     perror("Failed to synchronize file data");
        //     close(fd);
        //     return 1;
        // }
        ++ct;
        if(ct > 100)break;
    }
    // 在这里执行你的操作，可以是任何你想要测量时间的代码块
    
    // 记录结束时间
    end_time = clock();

    // 计算经过的时间（以秒为单位）
    elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    close(in);
    close(fd);
    // 输出经过的时间
    printf("程序执行时间: %.3f 秒\n", elapsed_time);

    return 0;
}