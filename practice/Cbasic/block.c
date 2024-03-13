#include <stdio.h>
#include <stdlib.h>
int main(){
    void* sp;
 
    // 使用內聯彙編獲取stack pointer
    __asm__("mov %%rsp, %0" : "=r"(sp));
    printf("     initial stack pointer: %p\n", sp);

    int *a;
    __asm__("mov %%rsp, %0" : "=r"(sp));
    printf("   declare a stack pointer: %p\n", sp);

    {
        int p;
        __asm__("mov %%rsp, %0" : "=r"(sp));
        printf("    in block stack pointer: %p\n", sp);

    }
    __asm__("mov %%rsp, %0" : "=r"(sp));
    printf("out of block stack pointer: %p\n", sp);

    return 0;
}