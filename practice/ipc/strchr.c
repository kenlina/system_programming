#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(){
    char *str = "/usr/bin/more";
    char *ptr;
    ptr = strrchr(str,'/');
    printf("%s\n",++ptr);
    return 0;
}