#include <unistd.h>
#include <stdio.h>
#include <signal.h>

int main(){
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGKILL);
    // SIGKILL沒辦法被block ignore catch
    if(sigprocmask(SIG_BLOCK, &mask, NULL) < 0){
        printf("block error\n");
        return -1;
    }
    sleep(200);
    return 0;
}
