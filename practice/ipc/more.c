#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define DEF_PAGER "/bin/more"

int main(int argc, char *argv[]){
    int n;
    int fd[2];
    pid_t pid;
    char *pager, *argv0;
    char line[1024];
    FILE *fp;


    if(argc!=2){
        printf("usage: ./a.out <pathname>\n");
        return 1;
    }

    if((fp=fopen(argv[1],"r")) == NULL){
        printf("fopen error\n");
        return 1;
    }
    if(pipe(fd)<0){
        printf("pipe error\n");
        return 1;
    }

    if((pid=fork())<0){
        printf("fork error\n");
        return 1;
    }
    else if (pid>0){
        close(fd[0]);
        while(fgets(line,1024,fp) != NULL){
            n = strlen(line);
            if(write(fd[1], line, n) != n){
                printf("write error\n");
                return 1;
            }
        }
        if(ferror(fp)){
            printf("fgets error\n");
            return 1;
        }
        close(fd[1]);
        if(waitpid(pid, NULL, 0) < 0){
            printf("waitpid error\n");
            return 1;
        }
        exit(0);
    }
    else{
        close(fd[1]);
        if(fd[0]!=STDIN_FILENO){
            if(dup2(fd[0], STDIN_FILENO) != STDIN_FILENO){
                printf("dup2 error\n");
                return 1;
            }
            close(fd[0]);
        }

        if((pager = getenv("PAGER")) == NULL) 
            pager = DEF_PAGER;

        if((argv0 = strrchr(pager, '/')) != NULL)
            ++argv0;
        else
            argv0 = pager;

        fprintf(stdout,"pager is %s\n", pager);
        if(execl(pager, argv0, NULL) < 0){
            printf("execl error for %s\n", pager);
            return 1;
        }
    }
    exit(0);
}
