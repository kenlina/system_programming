#include <unistd.h>
#include <stdio.h>
#include <sys/times.h>
#include <fcntl.h>
#include <stdlib.h>
void p_times(clock_t real, struct tms *tstart, struct tms *tend){
    long clktck = sysconf(_SC_CLK_TCK);
    printf(" real: %7.3f\n", real/(double)clktck);
    printf(" user: %7.3f\n", (tend->tms_utime-tstart->tms_utime)/(double)clktck);
    printf("  sys: %7.3f\n", (tend->tms_stime-tstart->tms_stime)/(double)clktck);
}
int main(){
    struct tms tstart, tend;
    clock_t start, end;
    start = times(&tstart);

    for(int i = 0; i < 80000; ++i){
        int fd = open("tmp", O_WRONLY);
        if(fd == -1){
            printf("open error\n");
            return -1;
        }
        if(write(fd,"wedwd",6) < 0){
            printf("write error\n");
            return -1;
        }
        close(fd);
    }
    
    end = times(&tend);
    p_times(end-start, &tstart, &tend);
    return 0;
}

