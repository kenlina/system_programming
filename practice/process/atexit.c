#include <stdlib.h>
#include <stdio.h>
void exit1();
void exit2();

int main(){
    if (atexit(exit2) != 0){
        printf("can't register exit2\n");
        exit(1);
    }
    if (atexit(exit1) != 0){
        printf("can't register exit1\n");
        exit(1);
    }
    if (atexit(exit1) != 0){
        printf("can't register exit1\n");
        exit(1);
    }
    printf("main is done\n");
    return 0;
}

void exit1(){
    printf("first exit handler\n");
}
void exit2(){
    printf("second exit handler\n");
}