#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define rwrwrw (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)
int main(){
    umask(0);
    if(creat("foo",rwrwrw) < 0){
        printf("create foo error\n");
    }
    
    umask(S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
    if(creat("bar",rwrwrw) < 0){
        printf("create bar erroe\n");
    }
    return 0;
}