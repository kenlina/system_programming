#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#ifdef PATH_MAX
static int pathmax = PATH_MAX;
#else
static int pathmax = 0;
#endif

int main(){
    errno = 0;
    if((pathmax = pathconf("/",_PC_PATH_MAX)) < 0){
        if(errno == 0)
            printf("indeterminate\n");
        else
            printf("pathconf error\n"); 
    }
    printf("pathmax = %d\n", pathmax);
    if(sysconf(_SC_CHILD_MAX) < 0){
        printf("sysconf error\n");
    }
    else{
        printf("max child: %ld\n", sysconf(_SC_CHILD_MAX));
    }
    if(sysconf(_SC_OPEN_MAX) < 0){
        printf("sysconf error\n");
    }
    else{
        printf("max open: %ld\n", sysconf(_SC_OPEN_MAX));
    }
    return 0;
}