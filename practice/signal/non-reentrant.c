#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <pwd.h>
unsigned int a = 0;

void my_alarm(int signo){
    signal(SIGINT, my_alarm);
    printf(
        "signal handler\n"
    );
    for( int i = 0; i < 500; ++i) ++a;
}

int main(){
    signal(SIGINT, my_alarm);
    for(;;){
        a = 0;
        for( int i = 0; i < 1000000000; ++i) ++a;
        printf("%d\n" , a);
    }
    return 0;
}