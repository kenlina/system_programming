#include <unistd.h>
#include <stdio.h>
int main(){
    char ptr[100];
    
    if(chdir("/lib") < 0){
        printf("chdir error\n");
        return -1;
    }
    if ( getcwd(ptr,100) < 0 ){
        printf("getcwd error\n");
        return -1;
    }
    printf("cwd = %s\n", ptr);
    return 0;
}