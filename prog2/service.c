#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "util.h"

#define ERR_EXIT(s) perror(s), exit(errno);

static unsigned long secret;
static char service_name[MAX_SERVICE_NAME_LEN];

static inline bool is_manager() {
    return strcmp(service_name, "Manager") == 0;
}

void print_not_exist(char *service_name) {
    printf("%s doesn't exist\n", service_name);
}

void print_receive_command(char *service_name, char *cmd) {
    printf("%s has received %s\n", service_name, cmd);
}

void print_spawn(char *parent_name, char *child_name) {
    printf("%s has spawned a new service %s\n", parent_name, child_name);
}

void print_kill(char *target_name, int decendents_num) {
    printf("%s and %d child services are killed\n", target_name, decendents_num);
}

void print_acquire_secret(char *service_a, char *service_b, unsigned long secret) {
    printf("%s has acquired a new secret from %s, value: %lu\n", service_a, service_b, secret);
}

void print_exchange(char *service_a, char *service_b) {
    printf("%s and %s have exchanged their secrets\n", service_a, service_b);
}

/* 自定義structure */
typedef struct child{
    service info;  /* child的info */
    struct child *next;
} child;           /* child的ll */

static int pfd1[2], pfd2[2]; 
/*
   pipe的fd 
   pfd1是 parent寫入 child讀
   pfd2是 child寫入  parent讀   
*/

/* 自定義函數 */
void deleteChild(child** head_ref, char *target1) {
    // 存儲當前節點和前一個節點的指針
    child* current = *head_ref;
    child* prev = NULL;

    // 如果要刪除的節點是頭節點，則更新頭指針
    if (current != NULL && strcmp(current->info.name, target1) == 0) {
        *head_ref = current->next;
        free(current);
        return;
    }

    // 搜索要刪除的節點，同時維護前一個節點的指針
    while (current != NULL && strcmp(current->info.name, target1) != 0) {
        prev = current;
        current = current->next;
    }

    // 如果找到要刪除的節點，則將前一個節點的next指針指向下一個節點
    if (current != NULL) {
        prev->next = current->next;
        free(current);
    }
}

void parse_cmd(char *str, char cmd[], char target1[], char target2[]){
    char *ptr = str;
    int idx = 0;
    while( *ptr != ' ' ){    /* 切出cmd */
        cmd[idx++] = *ptr++;
    }
    cmd[idx] = '\0';
    ++ptr;
    idx = 0;
    while( *ptr != ' ' && *ptr != '\0'){    /* 切出target1 */
        target1[idx++] = *ptr++;
    }
    target1[idx] = '\0';
    if( *ptr == '\0' ){      /* 沒有target2 */
        target2[0] = '\0';
        return;
    }

    ++ptr;
    idx = 0;
    while( *ptr != '\0'){    /* 切出target2 */
        target2[idx++] = *ptr++;
    }
    target2[idx] = '\0';
}
void mkpipe(){
    if( pipe2(pfd1, O_CLOEXEC) < 0 || pipe2(pfd2, O_CLOEXEC) < 0 )
        ERR_EXIT("pipe2");
    /* 
    如果pipe開的fd是3或4就要把他的CLOEXEC關掉 
    不然chile process在exec前如果把pipe的fd都導向3號4號的話
    在exec之後這些被導向的fd都會被關掉
    */
    if(pfd1[0] == 3){               
        int flags;
        if( ( flags = fcntl(3, F_GETFD) ) < 0) 
            ERR_EXIT("fcntl: F_GETFD");
        flags &= ~FD_CLOEXEC;
        if( fcntl(3, F_SETFD, flags) < 0 ) 
            ERR_EXIT("fcntl: F_SETFD");
    }
    if(pfd1[1] == 4){
        int flags;
        if( ( flags = fcntl(4, F_GETFD) ) < 0) 
            ERR_EXIT("fcntl: F_GETFD");
        flags &= ~FD_CLOEXEC;
        if( fcntl(4, F_SETFD, flags) < 0 ) 
            ERR_EXIT("fcntl: F_SETFD");
    }
}
void Spawn(){

}

/*
    Manager要印：
    1. 新service被spawn後要印 "[parent_service] has spawned a new service [new_child_service]", 要等Service Creation Message完
    2. 如果spawn或kill指令的parent不存在 "[parent_service] doesn't exist"
    3. 當一個service掛了 "[service] and [number of descendents] child services are killed"
    4. 等exchange各自都印完且關掉FIFOs後 "[service_a] and [service_b] have exchanged their secrets"

    每個process要印：
    1. 當自己出生 "[service_name] has been spawned, pid: [service_pid], secret: [service_secret]"
    2. 收到command "[service_name] has received [command]"
    3. 收到exchange的secret "[service_a] has acquired a new secret from [service_b], value: [service_b_secret]"
*/


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: ./service [service_name]\n");
        return 0;
    }

    pid_t pid = getpid();        
    srand(pid);
    secret = rand();
    /* 
     * prevent buffered I/O
     * equivalent to fflush() after each stdout
     */
    setvbuf(stdout, NULL, _IONBF, 0);

    strncpy(service_name, argv[1], MAX_SERVICE_NAME_LEN);

    printf("%s has been spawned, pid: %d, secret: %lu\n", service_name, pid, secret);
    /* 
        如果不是Manager的話代表你是被spawn出來的 
        你要告訴你爸你印完Service Creation Message了
        這樣你爸才往上告訴Manager讓它可以印"[parent_service] has spawned a new service [new_child_service]"
    */
    if( !is_manager() ){    
        if( write(PARENT_WRITE_FD, "spawnOK\0", 8) < 0)
            ERR_EXIT("write");
    }
    
    char input[MAX_CMD_LEN + 1];

    char cmd[MAX_CMD_LEN],               /* 單純指令(spawn, kill, exchange) */
         target1[MAX_SERVICE_NAME_LEN],  /* 指令第一個參數 */
         target2[MAX_SERVICE_NAME_LEN];  /* 指令第二個參數 */

    child *head = NULL;
    
    while (1){
        if(is_manager()){               /* Manager才會執行 */
            input[0] = '\0';
            fgets(input, MAX_CMD_LEN + 1, stdin);
            input[strlen(input) - 1] = '\0';    /* 把fgets讀到的換行符拿掉 */
        }
        else{                          /* 其他process做這裡 */
            input[0] = '\0';
            if( read(PARENT_READ_FD, input, MAX_CMD_LEN) < 0 )
                ERR_EXIT("read");
        }

        parse_cmd(input, cmd, target1, target2);
        if( ! (strcmp(target2, "NoPrint") == 0) ){
            print_receive_command(service_name, input);
        }

        /* 是target1 或是 exchange指令的target2 */
        if ( strcmp(target1, service_name) == 0 || ( strcmp(cmd, "exchange") == 0 && strcmp(target2, service_name) == 0 ) ){   
            /* 你有父親且你不是收到NoPrint的kill指令且不是exchange的話 -> 傳給父親"Mine" */
            if( !is_manager() && (strcmp(target2, "NoPrint") != 0) && (strcmp(cmd, "exchange") != 0) ){                 
                if( write(PARENT_WRITE_FD, "Mine\0", 5) < 0)
                    ERR_EXIT("write");
            }
            /* 處理spawn */
            if( strcmp(cmd, "spawn") == 0){          
                //fprintf(stderr, "processing spawn call\n");
                mkpipe();
                if( ( pid = fork() ) < 0 ){ ERR_EXIT("fork"); }
                else if( pid == 0){                  /* child */
                    close(pfd1[1]);
                    close(pfd2[0]);
                    if(pfd1[0] != 3){
                        dup2(pfd1[0], 3);
                        close(pfd1[0]);
                    }
                    if(pfd2[1] != 4){
                        dup2(pfd2[1], 4);
                        close(pfd2[1]);
                    }
                    execl("./service", "service", target2, NULL);
                }
                else{                                /* parent */
                    close(pfd1[0]);
                    close(pfd2[1]);
                    child* tmp = (child*)malloc(sizeof(child));
                    strncpy(tmp->info.name, target2, MAX_SERVICE_NAME_LEN);
                    tmp->info.pid = pid;
                    tmp->info.read_fd = pfd2[0];
                    tmp->info.write_fd = pfd1[1];
                    tmp->next = NULL;
                    if( head == NULL ){ head = tmp; }
                    else{
                        child *ptr = head;
                        while (ptr->next != NULL){
                            ptr = ptr->next;
                        }
                        ptr->next = tmp;
                    }
                    char check[15];
                    /* 父親生完小孩後要等小孩回傳spawnOK才知道它印完訊息了 */
         /* 收到child找到target1的回覆 */           if( read(pfd2[0], check, 8) < 0)
                            ERR_EXIT("read");
                    if( strcmp(check, "spawnOK") == 0 ){
                        if( is_manager() ){         /* 如果是Manager就印提示訊息 */
                            print_spawn(target1, target2);
                        }
                        else{                       /* 如果有老爸就傳給Ta */
                            if( write(PARENT_WRITE_FD, "spawnOK\0", 8) < 0)
                                ERR_EXIT("write");
                        }
                    }
                }
            }
            /* 結束處理spawn */

            /* 處理kill */
            if( strcmp(cmd, "kill") == 0){           
                //fprintf(stderr, "processing kill call\n");
                child* ptr = head;
                int num = (strcmp(target2, "NoPrint") == 0) ? 1 : 0; /* 如果kill目標是自己的話 不用把自己加上去死亡數*/
                char pass_cmd[128];
                while (ptr != NULL){
                    pass_cmd[0] = '\0';
                    snprintf(pass_cmd, sizeof(pass_cmd), "kill %s NoPrint", ptr->info.name);
                    /* 
                        pass_cmd的格式會長這樣 "kill service_name NoPrint" 最後的參數指示不用印出 
                        kill目標收到後就往下傳kill NoPrint版本
                        他的小孩收到都要繼續kill 但不用印出收到的訊息
                    */
                    if( write(ptr->info.write_fd, pass_cmd, strlen(pass_cmd) + 1) < 0 )
                        ERR_EXIT("write");
                    read(ptr->info.read_fd, pass_cmd, 128);
                    num += atoi(pass_cmd);
                    waitpid(ptr->info.pid, NULL, 0);
                    child *tmp = ptr;
                    ptr = ptr->next;
                    free(tmp);
                }
                if( !is_manager() ){    /* 如果不是Manager的話就往上傳底下死了幾個人 */
                    sprintf(pass_cmd, "%d", num);
                    if( write(PARENT_WRITE_FD, pass_cmd, strlen(pass_cmd) + 1) < 0 )
                        ERR_EXIT("write");
                }
                else{                 /* 如果是Manager的話就印出死亡訊息 */
                    print_kill(target1, num);
                }
                return 0;
            }
            /* 結束處理kill */

            /* 處理exchange */
            if( strcmp(cmd, "exchange") == 0 ){     
                //fprintf(stderr, "processing exchange call\n");
                char buf[20];
                /* 如果是Manager的話 */
                if( is_manager() ){    
                    child *ptr = head;
                    char check[15];
                    check[0] = '\0';
                    int down = 0;
                    child *second_target = NULL;
                    while (ptr != NULL){
                        /* 把指令傳給child */
                        write(ptr->info.write_fd, input, strlen(input) + 1 );
                        
                        /* 下面那行read是讀ok用的 拿來切割兩次write*/
                        read(ptr->info.read_fd, check, sizeof(check));
                        
                        /* 告訴child當下received數量 */
                        sprintf(check,"%d", down + 1); // 1是你自己
                        write(ptr->info.write_fd, check, strlen(check) + 1 );
                        
                        /* 讀取child回傳他的target數量 */
                        read(ptr->info.read_fd, check, sizeof(check));
                        down += atoi(check);
                        if( down == 1 ){
                            second_target = ptr;
                        }
                        if( (1 + down) == 2) break;
                        ptr = ptr->next;
                    }
                    char fifo_name_w[MAX_FIFO_NAME_LEN];
                    char fifo_name_r[MAX_FIFO_NAME_LEN];
                    fifo_name_w[0] = '\0'; 
                    fifo_name_r[0] = '\0';   
                    int fifo_w, fifo_r;
                    int target_secret; 
                    /* 如果你是target1 */
                    if( strcmp(service_name, target1) == 0 ){
                        snprintf(fifo_name_w, MAX_FIFO_NAME_LEN, "./%s_to_%s.fifo", target1, target2);
                        snprintf(fifo_name_r, MAX_FIFO_NAME_LEN, "./%s_to_%s.fifo", target2, target1);
                    }
                    /* 你是target2 */
                    else{
                        snprintf(fifo_name_w, MAX_FIFO_NAME_LEN, "./%s_to_%s.fifo", target2, target1);
                        snprintf(fifo_name_r, MAX_FIFO_NAME_LEN, "./%s_to_%s.fifo", target1, target2);
                    }
                    /* 建立fifo */
                    if (mkfifo(fifo_name_w, 0666) == -1){
                        if (errno != EEXIST) {
                            // 如果錯誤不是因為檔案已存在
                            ERR_EXIT("mkfifo");
                        } else {
                            // 檔案已存在的情況，可以在這裡處理，或者什麼都不做
                        }
                    }
                    if (mkfifo(fifo_name_r, 0666) == -1){
                        if (errno != EEXIST) {
                            // 如果錯誤不是因為檔案已存在
                            ERR_EXIT("mkfifo");
                        } else {
                            // 檔案已存在的情況，可以在這裡處理，或者什麼都不做
                        }
                    }
                    /* Manager一定是第一個received的 */
                    fifo_w = open(fifo_name_w, O_WRONLY);
                    if (fifo_w < 0) ERR_EXIT("open");
                    sprintf(buf, "%ld", secret);
                    write(fifo_w, buf, strlen(buf) + 1);
                    fifo_r = open(fifo_name_r, O_RDONLY);
                    if (fifo_r < 0) ERR_EXIT("open");
                    read(fifo_r, buf, sizeof(buf));
                    target_secret = atoi(buf);
                    /* 如果你是target1你就先print然後用fifo跟對方說你好了 */
                    if( strcmp(service_name, target1) == 0 ){
                        print_acquire_secret(service_name, target2, target_secret);
                        write(fifo_w, "ok\0", 3);
                    }
                    /* 你是target2就等到死 */
                    else{
                        read(fifo_r, buf, sizeof(buf));
                        if( strcmp(buf, "ok") == 0 ){
                            print_acquire_secret(service_name, target1, target_secret);
                        }
                    }
                    close(fifo_w);
                    close(fifo_r);
                    secret = target_secret;
                    
                    /* 刪除fifo檔 */
                    if (unlink(fifo_name_r) == -1) {
                        if (errno != ENOENT) {
                            // 如果錯誤不是因為檔案不存在
                            ERR_EXIT("unlink");
                        } else {
                            // 檔案已經不存在
                        }
                    }
                    if (unlink(fifo_name_w) == -1) {
                        if (errno != ENOENT) {
                            // 如果錯誤不是因為檔案不存在
                            ERR_EXIT("unlink");
                        } else {
                            // 檔案已經不存在
                        }
                    }
                    write(second_target->info.write_fd, "split", 6);
                    read(second_target->info.read_fd, check, sizeof(check));
                    print_exchange(target1, target2);
                }
                /* 如果不是Manager的話 */
                else{     
                    write(PARENT_WRITE_FD, "ok\0", 3);
                    buf[0] = '\0';
                    read(PARENT_READ_FD, buf, 20);   
                    int received = atoi(buf); 
                    int down = 0;
                    child *another_target = NULL;
                    /* 如果你是第一個收到那你的小孩有可能是target 所以要往下找有沒有target */
                    if( received == 0){
                        child *ptr = head;
                        char check[15];
                        check[0] = '\0';
                        while (ptr != NULL){
                            /* 把指令傳給child */
                            write(ptr->info.write_fd, input, strlen(input) + 1 );
                            
                            /* 下面那行read是讀ok用的 拿來切割兩次write*/
                            read(ptr->info.read_fd, check, sizeof(check));
                            
                            /* 告訴child當下received數量 */
                            sprintf(check,"%d", down + 1); // 1是你自己
                            write(ptr->info.write_fd, check, strlen(check) + 1 );
                            
                            /* 讀取child回傳他的target數量 */
                            read(ptr->info.read_fd, check, sizeof(check));
                            down += atoi(check);
                            if ( down == 1 ) another_target = ptr;
                            if( (1 + down) == 2) break;
                            ptr = ptr->next;
                        }
                    }
                    /* 此時已經兩個target都找到了 */
                    if( received == 1 ){
                        /* 你就不用再往下找target了 因為你就是第二個了 */
                    }
                    sprintf(buf,"%d", down + 1); // 1是你自己
                    write(PARENT_WRITE_FD, buf, strlen(buf) + 1 );
                    char fifo_name_w[MAX_FIFO_NAME_LEN];
                    char fifo_name_r[MAX_FIFO_NAME_LEN];
                    fifo_name_w[0] = '\0'; 
                    fifo_name_r[0] = '\0';   
                    int fifo_w, fifo_r;
                    int target_secret; 
                    /* 如果你是target1 */
                    if( strcmp(service_name, target1) == 0 ){
                        snprintf(fifo_name_w, MAX_FIFO_NAME_LEN, "./%s_to_%s.fifo", target1, target2);
                        snprintf(fifo_name_r, MAX_FIFO_NAME_LEN, "./%s_to_%s.fifo", target2, target1);
                    }
                    /* 你是target2 */
                    else{
                        snprintf(fifo_name_w, MAX_FIFO_NAME_LEN, "./%s_to_%s.fifo", target2, target1);
                        snprintf(fifo_name_r, MAX_FIFO_NAME_LEN, "./%s_to_%s.fifo", target1, target2);
                    }
                    /* 建立fifo */
                    if (mkfifo(fifo_name_w, 0666) == -1){
                        if (errno != EEXIST) {
                            // 如果錯誤不是因為檔案已存在
                            ERR_EXIT("mkfifo");
                        } else {
                            // 檔案已存在的情況，可以在這裡處理，或者什麼都不做
                        }
                    }
                    if (mkfifo(fifo_name_r, 0666) == -1){
                        if (errno != EEXIST) {
                            // 如果錯誤不是因為檔案已存在
                            ERR_EXIT("mkfifo");
                        } else {
                            // 檔案已存在的情況，可以在這裡處理，或者什麼都不做
                        }
                    }
                    /* 你是第一個收到的 第二個target還沒收到 */
                    // 先寫再讀
                    if(received == 0){
                        fifo_w = open(fifo_name_w, O_WRONLY);
                        if (fifo_w < 0) ERR_EXIT("open");
                        sprintf(buf, "%ld", secret);
                        write(fifo_w, buf, strlen(buf) + 1);
                        fifo_r = open(fifo_name_r, O_RDONLY);
                        if (fifo_r < 0) ERR_EXIT("open");
                        read(fifo_r, buf, sizeof(buf));
                        target_secret = atoi(buf);
                        /* 如果你是target1你就先print然後用fifo跟對方說你好了 */
                        if( strcmp(service_name, target1) == 0 ){
                            print_acquire_secret(service_name, target2, target_secret);
                            write(fifo_w, "ok\0", 3);
                        }
                        /* 你是target2就等到死 */
                        else{
                            read(fifo_r, buf, sizeof(buf));
                            if( strcmp(buf, "ok") == 0 ){
                                print_acquire_secret(service_name, target1, target_secret);
                            }
                        }
                        close(fifo_w);
                        close(fifo_r);
                    }
                    /* 你是第二個收到了 在你前面已經有人收到了 */
                    // 先讀再寫
                    if(received == 1){
                        fifo_r = open(fifo_name_r, O_RDONLY);
                        if (fifo_r < 0) ERR_EXIT("open");
                        read(fifo_r, buf, sizeof(buf));
                        target_secret = atoi(buf);
                        fifo_w = open(fifo_name_w, O_WRONLY);
                        if (fifo_w < 0) ERR_EXIT("open");
                        sprintf(buf, "%ld", secret);
                        write(fifo_w, buf, strlen(buf) + 1);
                        if( strcmp(service_name, target1) == 0 ){
                            print_acquire_secret(service_name, target2, target_secret);
                            write(fifo_w, "ok\0", 3);
                        }
                        /* 你是target2就等到死 */
                        else{
                            read(fifo_r, buf, sizeof(buf));
                            if( strcmp(buf, "ok") == 0 ){
                                print_acquire_secret(service_name, target1, target_secret);
                            }
                        }
                        close(fifo_r);
                        close(fifo_w);
                    }
                    secret = target_secret;
                    
                    /* 刪除fifo檔 */
                    if (unlink(fifo_name_r) == -1) {
                        if (errno != ENOENT) {
                            // 如果錯誤不是因為檔案不存在
                            ERR_EXIT("unlink");
                        } else {
                            // 檔案已經不存在
                        }
                    }
                    if (unlink(fifo_name_w) == -1) {
                        if (errno != ENOENT) {
                            // 如果錯誤不是因為檔案不存在
                            ERR_EXIT("unlink");
                        } else {
                            // 檔案已經不存在
                        }
                    }
                    
                    /* 你是第一個收到完成所有動作後要往上傳 */
                    if ( received == 0){
                        read(PARENT_READ_FD, buf, sizeof(buf));
                        write(PARENT_WRITE_FD, "exchangeOK\0", 11);
                        
                        if(another_target != NULL){
                            write(another_target->info.write_fd, "ok",3);
                            read(another_target->info.read_fd, buf, sizeof(buf));
                            write(PARENT_WRITE_FD, buf, strlen(buf) + 1);
                        }
                    }
                    if( received == 1 ){
                        read(PARENT_READ_FD, buf, sizeof(buf));
                        write(PARENT_WRITE_FD, "exchangeOK\0", 11);
                    }
                }
            }
            /* 結束處理exchange */
            
            /* debug 列出所有child */
            if( strcmp(cmd, "p") == 0 ){
                fprintf(stderr, "processing p call\n");
                child* ptr = head;
                while( ptr != NULL){
                    fprintf(stderr,
                    "name:%6s, pid: %6d, read: %3d, write: %3d\n",
                    ptr->info.name, ptr->info.pid, ptr->info.read_fd, ptr->info.write_fd);
                    ptr = ptr->next;
                }
                if ( !is_manager() ){             /* 不是Manager的話就往上傳完成p*/
                    if( write(PARENT_WRITE_FD, "pOK\0", 4) < 0)
                        ERR_EXIT("write");
                }
            }
            
        }
        else{   /* 不是target1 或exchange的target2，傳下去 */
            child* ptr = head; 
            /* 如果不是exchange指令的話只會有一個目標要傳 */
            if( strcmp(cmd, "exchange") !=0 ){
                bool stop_pass = 0;      /* 要不要停止傳下去 */                           
                while (ptr != NULL){
                    write(ptr->info.write_fd, input, strlen(input) + 1 );
                    /* 
                        check讀到的是判斷要不要繼續傳下去的回應 

                        Mine: 代表這個child底下有target1
                            -> 不用繼續傳下去了，如果有父親傳給父親Mine -> early stop

                        NotMine: 代表這個child不是target1
                            -> 要繼續傳下去
                    */
                    char check[15] = {};
                    read(ptr->info.read_fd, check, 15);
                    if( strcmp(check, "Mine") == 0 ){    /* 收到child找到target1的回覆 */  
                        stop_pass = 1;
                        if( !is_manager() ){             /* 有父親的處理 */
                            if( write(PARENT_WRITE_FD, "Mine\0", 5) < 0) /* 告訴它老爸底下有人是target1 */
                                ERR_EXIT("write");
                            if( read(ptr->info.read_fd, check, 15) < 0)   /* 第二次讀回應看target1處理的狀況如何 */
                                ERR_EXIT("read");

                            if( strcmp(check, "spawnOK") == 0 ){ /* spawn的回應 */
                                if( write(PARENT_WRITE_FD, "spawnOK\0", 8) < 0)
                                    ERR_EXIT("write");
                            }
                            else if(strcmp(check, "pOK") == 0){  /* p的回應 */
                                if( write(PARENT_WRITE_FD, "pOK\0", 4) < 0)
                                    ERR_EXIT("write");
                            }
                            else{                               /* kill的回應 */
                                if( strcmp(ptr->info.name, target1) == 0 ){  /* 被殺的是自己小孩的話就把Ta從list拿掉 */
                                    close(ptr->info.read_fd);
                                    close(ptr->info.write_fd);
                                    deleteChild(&head, target1);
                                }
                                if( write(PARENT_WRITE_FD, check, strlen(check) + 1) < 0)
                                    ERR_EXIT("write");
                            }
                        }
                        else{                            /* Manager處理找到target1 */
                            if( read(ptr->info.read_fd, check, 15) < 0)
                                ERR_EXIT("read");

                            if( strcmp(check, "spawnOK") == 0 ){ /* spawn的回應 */
                                print_spawn(target1, target2);
                            }
                            else if(strcmp(check, "exchangeOK") == 0){  /* exchange的回應 */

                            }
                            else if(strcmp(check, "pOK") == 0){  /* p的回應 */

                            }
                            else{                               /* kill的回應 kill回傳的是字串型態的死亡數 */
                                if( strcmp(ptr->info.name, target1) == 0 ){  /* 被殺的是自己小孩的話就把Ta從list拿掉 */
                                    close(ptr->info.read_fd);
                                    close(ptr->info.write_fd);
                                    deleteChild(&head, target1);
                                }
                                int num = atoi(check);
                                print_kill(target1, num);
                            }
                        }
                        break;
                    }
                    /* 此child沒有找到target1的回覆 */
                    if( strcmp(check, "NotMine") == 0 ){ /* 繼續傳下去 */ }
                    ptr = ptr->next;
                }
                /* 你有父親但你的小孩沒有target1 -> 繼續傳 */ 
                if( !is_manager() && !stop_pass ){                 
                    if( write(PARENT_WRITE_FD, "NotMine\0", 8) < 0)
                        ERR_EXIT("write");
                }
                /* Manager的小孩沒有target1 -> 回傳不存在 */
                if( is_manager() && !stop_pass ){         
                    print_not_exist(target1);
                }
            }
            /* 如果是exchange指令的話 */
            else{
                char check[15];
                check[0] = '\0';
                int received = 0; /* 如果是manager到這裡代表它不是target received為0 */
                if( !is_manager() ){
                    /* 拿來分隔兩段write */
                    write(PARENT_WRITE_FD, "ok\0", 3);
                    read(PARENT_READ_FD, check, 15);
                    received = atoi(check);
                }
                /* down是這process底下的child的target數 */
                int down = 0;
                /* no_received是拿來找第一個收到的target */
                bool no_received = (received == 0) ? 1 : 0;

                /* 如果你的後代有第一個收到的target 那你最後就要多read一次完成刪除fifo的回應*/
                bool have_first_received = 0;
                bool have_second_received = 0;
                child *first_target = NULL; /* 第一個收到 */
                child *second_target = NULL;/* 第二個收到 */
                while (ptr != NULL){
                    /* 把指令傳給child */
                    write(ptr->info.write_fd, input, strlen(input) + 1 );
                    read(ptr->info.read_fd, check, sizeof(check));
                    /* 告訴child當下received數量 */
                    sprintf(check,"%d", down + received);
                    write(ptr->info.write_fd, check, strlen(check) + 1 );
                    /* 讀取child回傳他的target數量 */
                    read(ptr->info.read_fd, check, sizeof(check));
                    down += atoi(check);
                    
                    /* down不是零且是第一個receive的 */
                    if( down > 0 && no_received ){
                        if( down == 1 ){
                            no_received = 0;
                            have_first_received = 1;
                            first_target = ptr;
                        }
                        if( down == 2 ){
                            no_received = 0;
                            have_first_received = 1;
                            first_target = ptr;
                            have_second_received = 1;
                            second_target = ptr;
                        }
                    }
                    /* down不是零且不是第一個receive的 */
                    else if ( (down == 2 &&  received == 0) || (down == 1 && received == 1) ) {
                        have_second_received = 1;
                        second_target = ptr;
                    }

                    if( (received + down) == 2) break;
                    ptr = ptr->next;
                }
                /* 不是manager就往上傳數量 */
                if ( !is_manager() ){    
                    sprintf(check, "%d", down);
                    if( write(PARENT_WRITE_FD, check, strlen(check) + 1) < 0)
                        ERR_EXIT("write");
                }
                
                /* 如果你底下兩個都有 */
                if( have_first_received && have_second_received && first_target == second_target ){
                    if( is_manager() ){
                        write(first_target->info.write_fd, "split", 6);
                        read(first_target->info.read_fd, check, sizeof(check));
                    }
                    else{
                        read(PARENT_READ_FD, check, sizeof(check));
                        write(first_target->info.write_fd, "split", 6);
                        read(first_target->info.read_fd, check, sizeof(check));
                        write( PARENT_WRITE_FD, check, strlen(check) + 1);
                    }
                }
                /* 如果你底下有第一個收到的target你就要等它完成的回應 */
                else {
                    if ( have_first_received ){
                        if( is_manager() ){
                            write(first_target->info.write_fd, "split", 6);
                            read(first_target->info.read_fd, check, sizeof(check));
                        }
                        else{
                            read(PARENT_READ_FD, check, sizeof(check));
                            write(first_target->info.write_fd, "split", 6);
                            read(first_target->info.read_fd, check, sizeof(check));
                            write( PARENT_WRITE_FD, check, strlen(check) + 1);
                        }
                        
                    }
                    if ( have_second_received ){
                        if( is_manager() ){
                            write(second_target->info.write_fd, "split", 6);
                        }
                        else{
                            read(PARENT_READ_FD, check, sizeof(check));
                            write(second_target->info.write_fd, "split", 6);
                            read(second_target->info.read_fd, check, sizeof(check));
                            write( PARENT_WRITE_FD, check, strlen(check) + 1);
                        }
                        
                    }
                }
                /* manager收到完成訊息 印出訊息 */
                if( is_manager() ){     
                    
                    read(second_target->info.read_fd, check, sizeof(check));
                              
                    if( strcmp(check, "exchangeOK") == 0)
                        print_exchange(target1, target2);
                }

            }
            
        }
        
    }
    return 0;
}
