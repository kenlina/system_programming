#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
static void output(char*);
int main(){
    pid_t pid;
    if((pid=fork())<0){
        printf("fork error\n");
        exit(0);
    }
    else if (pid == 0){
        output("output from child ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc\n");
    }
    else{
        output("output from parent pppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppp\n");
    }
    exit(0);
}

static void output(char *str){
    char *ptr;
    int c;
    setbuf(stdout, NULL);
    for (ptr = str; (c = *ptr++) != 0; ) putc(c, stdout);
}