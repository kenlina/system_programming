#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
static volatile sig_atomic_t sigflag;
static sigset_t new, old, zero;
void sig_usr(int signo){
    sigflag = 1;
}
void TELL_WAIT(){
    signal(SIGUSR1, sig_usr);
    signal(SIGUSR2, sig_usr);

    sigemptyset(&zero), sigemptyset(&new);
    sigaddset(&new, SIGUSR1);
    sigaddset(&new, SIGUSR2);

    sigprocmask(SIG_BLOCK, &new, &old);
}
void WAIT_CHILD(){
    while(sigflag == 0) sigsuspend(&zero);
    sigflag = 0;
    sigprocmask(SIG_SETMASK, &old, NULL);
}

void TELL_PARENT(pid_t pid){
    kill(pid, SIGUSR2);
}

static void output(char*);
int main(){
    pid_t pid;

    TELL_WAIT();

    if((pid=fork())<0){
        printf("fork error\n");
        exit(0);
    }
    else if (pid == 0){
        output("output from ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc\n");
        TELL_PARENT(getppid());
    }
    else{
        WAIT_CHILD();
        output("output from ppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppp\n");
    }
    exit(0);
}

static void output(char *str){
    char *ptr;
    int c;
    setbuf(stdout, NULL);
    for (ptr = str; (c = *ptr++) != 0; ) putc(c, stdout);
}