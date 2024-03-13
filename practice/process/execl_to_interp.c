#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <wait.h>
int main(){
    
    if(execl("/home/ken/Desktop/112_1/sp/practice/process/interp",
             "interp", "myarg1", "myarg2", NULL) < 0)
    {
        perror("execl");
        return -1;
    }
    
    return 0;
}