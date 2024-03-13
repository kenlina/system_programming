#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>

int main() {
    uid_t ruid, euid, suid;

    // 獲取real UID、effective UID和saved UID
    if (getresuid(&ruid, &euid, &suid) == -1) {
        perror("getresuid");
        return 1;
    }

    printf("after exec:\n");
    printf("Real UID: %d\n", ruid);
    printf("Effective UID: %d\n", euid);
    printf("Saved UID: %d\n", suid);
    printf("\n");
    

    return 0;
}