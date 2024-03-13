#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>
int main(int argc, char *argv[]) {
    uid_t ruid, euid, suid;


    // 獲取real UID、effective UID和saved UID
    if (getresuid(&ruid, &euid, &suid) == -1) {
        perror("getresuid");
        return 1;
    }
    printf("run:\n");
    printf("Real UID: %d\n", ruid);
    printf("Effective UID: %d\n", euid);
    printf("Saved UID: %d\n", suid);
    printf("\n");

    setuid(getuid());
    if (getresuid(&ruid, &euid, &suid) == -1) {
        perror("getresuid");
        return 1;
    }
    printf("run: after setuid(getuid());\n");
    printf("Real UID: %d\n", ruid);
    printf("Effective UID: %d\n", euid);
    printf("Saved UID: %d\n", suid);
    printf("\n");

    if(fork() == 0){
        if (getresuid(&ruid, &euid, &suid) == -1) {
            perror("getresuid");
            return 1;
        }
        printf("child:\n");
        printf("Real UID: %d\n", ruid);
        printf("Effective UID: %d\n", euid);
        printf("Saved UID: %d\n", suid);
        printf("\n");

        execl(argv[1], argv[1], NULL);
    }
    else{
        wait(NULL);
    }

    return 0;
}
