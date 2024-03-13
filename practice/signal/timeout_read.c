#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
void sig_alarm(int signo){
    // do nothing
}
char buf[BUFSIZ];
int main(){
    /* 
        signal函數在linux下會auto restart所以沒辦法做到timeout read
        改用sigaction才能把auto restart關掉
    */
    struct sigaction sa;
    sa.sa_handler = sig_alarm;
    sa.sa_flags = 0; // 確保不使用 SA_RESTART
    sigemptyset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, NULL);

    
    int r;
    alarm(5);
    if( (r = read(STDIN_FILENO, buf, 100)) < 0 ){
        if (errno == EINTR) {
            perror("read interrupted by signal");
        } 
        else {
            perror("read error");
        }
        return -1;
    }
    alarm(0);
    write(STDOUT_FILENO, buf , r);
    return 0;
}