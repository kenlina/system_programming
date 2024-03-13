#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
static void output(char*);
void TELL_WAIT();
void WAIT_PARENT();
void TELL_CHILD(pid_t);
void WAIT_CHILD();
void TELL_PARENT(pid_t);

static int pfd1[2], pfd2[2];

int main(){
    pid_t pid;
    printf("I am parent process, my id is %d\n", getpid());
    TELL_WAIT();
    printf("pfd1[0]:%d, pfd1[1]:%d\npfd2[0]:%d, pfd2[1]:%d\n", pfd1[0], pfd1[1], pfd2[0], pfd2[1]);
    
    if((pid=fork())<0){
        printf("fork error\n");
        exit(0);
    }
    else if (pid == 0){
        output("output from child\n");
        TELL_PARENT(getppid());
    }
    else{
        WAIT_CHILD();
        output("output from parent\n");
    }
    exit(0);
}

static void output(char *str){
    char *ptr;
    int c;
    setbuf(stdout, NULL);
    for (ptr = str; (c = *ptr++) != 0; ) putc(c, stdout);
}

void TELL_WAIT(){
    if(pipe(pfd1) < 0 || pipe(pfd2) < 0){
        printf("pipe error\n");
        exit(1);
    }
}

void WAIT_PARENT(){
    char c;
    if(read(pfd1[0], &c, 1) != 1){
        printf("read error\n");
        exit(1);
    }
    if(c != 'p'){
        printf("WAIT_PARENT: incorrect data\n");
        exit(1);
    }
}

void TELL_CHILD(pid_t pid){
    if(write(pfd1[1], "p", 1) != 1){
        printf("write error\n");
        exit(1);
    }
}

void WAIT_CHILD(){
    char c;
    if(read(pfd2[0], &c, 1) != 1){
        printf("read error\n");
        exit(1);
    }
    if(c != 'c'){
        printf("WAIT_CHILD: incorrect data\n");
        exit(1);
    }
}

void TELL_PARENT(pid_t pid){
    printf("I am child process, my parent id is %d\n", pid);
    if(write(pfd2[1], "c", 1) != 1){
        printf("write error\n");
        exit(1);
    }
}