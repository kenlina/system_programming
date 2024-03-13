#include "hw1.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <poll.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>

#define ERR_EXIT(a) do { perror(a); exit(1); } while(0)
#define BUFFER_SIZE 512
void pull_board(char *buf, int board_fd, struct pollfd *fds, int i);
typedef struct {
    char hostname[512];  // server's hostname
    unsigned short port;  // port to listen
    int listen_fd;  // fd to wait for a new connection
} server;

typedef struct {
    char host[512];  // client's host
    int conn_fd;  // fd to talk with client
    char buf[BUFFER_SIZE];  // data sent by/to client
    size_t buf_len;  // bytes used by buf
    int id;
} request;

server svr;  // server
request* requestP = NULL;  // point to a list of requests
int maxfd;  // size of open file descriptor table, size of request list

// initailize a server, exit for error
static void init_server(unsigned short port);

// initailize a request instance
static void init_request(request* reqP);

// free resources used by a request instance
static void free_request(request* reqP);

int last = 0;
bool writing[10] = {false};

int main(int argc, char** argv) {

    // Parse args.
    if (argc != 2) {
        ERR_EXIT("usage: [port]");
        exit(1);
    }

    struct sockaddr_in cliaddr;  // used by accept()
    int clilen;

    int conn_fd;  // fd for a new connection with client
    int file_fd;  // fd for file that we open for reading
    char buf[BUFFER_SIZE];
    int buf_len;

    // Initialize server
    init_server((unsigned short) atoi(argv[1]));

    // Loop for handling connections
    fprintf(stderr, "\nstarting on %.80s, port %d, fd %d, maxconn %d...\n", svr.hostname, svr.port, svr.listen_fd, maxfd);
    
    int board_fd;

    if((board_fd = openat(AT_FDCWD, "./BulletinBoard", O_RDWR | O_CREAT, 0644)) < 0)
        ERR_EXIT("openat");
    
    fprintf(stderr, "\nBulletinBoard fd: %d\n", board_fd);

    struct pollfd fds[21]; // 最多20個client加上一個listen_fd
    nfds_t nfds= 1;
    fds[0].fd = svr.listen_fd;
    fds[0].events = POLLIN;
    int cur_pollfd = 0;
    struct flock lock;
    setlinebuf(stdout);

    while (1) {
        int ret = poll(fds, nfds, -1);
        if (ret < 0) ERR_EXIT("poll");

        // TODO: Add IO multiplexing
        for( int i = 0; i < nfds; ++i){
            if(fds[i].revents & POLLIN){

                // Check new connection 
                if(fds[i].fd == svr.listen_fd){
                    clilen = sizeof(cliaddr);
                    conn_fd = accept(svr.listen_fd, (struct sockaddr*)&cliaddr, (socklen_t*)&clilen);
                    if (conn_fd < 0) {
                        if (errno == EINTR || errno == EAGAIN) continue;  // try again
                        if (errno == ENFILE) {
                            (void) fprintf(stderr, "out of file descriptor table ... (maxconn %d)\n", maxfd);
                            continue;
                        }
                        ERR_EXIT("accept");
                    }
                    
                    ++nfds;
                    ++cur_pollfd;
                    fds[cur_pollfd].fd = conn_fd;
                    fds[cur_pollfd].events = POLLIN; 
                    requestP[conn_fd].conn_fd = conn_fd;
                    strcpy(requestP[conn_fd].host, inet_ntoa(cliaddr.sin_addr));
                    fprintf(stderr, "getting a new request... fd %d from %s\n", conn_fd, requestP[conn_fd].host);
                }
                else{ // Not new connection, process request
                    ssize_t read_byte = read(fds[i].fd, requestP[fds[i].fd].buf, BUFFER_SIZE);
                    if(read_byte <= 0){
                        fprintf(stderr,"Reading from client error\n");
                        close(fds[i].fd);
                    } 
                    else{
                        if(strncmp(requestP[fds[i].fd].buf,"exit",4) == 0){
                            fprintf(stderr,"fd %d, ip %s exit\n", fds[i].fd, requestP[fds[i].fd].host);
                            close(fds[i].fd);
                            free_request(&requestP[fds[i].fd]);  
                            for(int j = i + 1; j <= cur_pollfd; ++j){
                                fds[j-1] = fds[j]; 
                            } 
                            --cur_pollfd;
                            --nfds;
                        }
                        if(strncmp(requestP[fds[i].fd].buf,"post",4) == 0){
                            strcpy(requestP[fds[i].fd].buf, "");
                            strcpy(requestP[fds[i].fd].buf, "post");
                            
                            lock.l_type = F_WRLCK;   // 設置為寫鎖定
                            lock.l_whence = SEEK_SET; // 從文件開始
                            lock.l_start = last * 25;        // 鎖定的起始位移
                            lock.l_len = 25;
                            
                            bool find = 0;
                            for(int time = 0; time < 10; ++time){
                                if(lseek(board_fd, last * 25 ,SEEK_SET) == -1) fprintf(stderr,"lseek error\n");
                                char test[2];
                                read(board_fd, test, 1);
                                if ( test[0] == '\0'){
                                    fprintf(stderr,"The record %d is empty\n", last);
                                    lock.l_start = last * 25;

                                    
                                    if(fcntl(board_fd, F_SETLK, &lock) == -1){
                                        fprintf(stderr,"The record %d is locked\n", last);
                                        ++last;
                                        last %= 10;
                                    }
                                    else{
                                        fprintf(stderr,"Locked the record %d\n", last);
                                        find = 1;
                                        break;
                                    }
                                }
                                else {
                                    fprintf(stderr,"The record %d is not empty\n", last);
                                    ++last;
                                    last %= 10; 
                                }
                            }
                            if ( !find ){
                                fprintf(stderr,"No empty record\n");
                                for(int time = 0; time < 10; ++time){
                                    lock.l_start = last * 25;
                                    if (fcntl(board_fd, F_SETLK, &lock) == -1){
                                        fprintf(stderr,"The record %d is locked\n", last);
                                        ++last;
                                        last %= 10; 
                                    }
                                    else{
                                        find = 1;
                                        fprintf(stderr,"Locked the record %d\n", last);
                                        break;
                                    }
                                    
                                }
                            }

                            // 檢查有沒有可寫入的record
                            if ( !find )  strcpy(requestP[fds[i].fd].buf, "failed");
                            else{
                                char laststr[3];
                                sprintf(laststr, "%d", last);
                                strcat(requestP[fds[i].fd].buf, laststr);
                                fprintf(stderr,"Record %d is being writing\n", last);
                                writing[last] = 1;
                                ++last;
                                last %= 10;
                                
                            }
                            fds[i].events |= POLLOUT;
                        }
                        if(strncmp(requestP[fds[i].fd].buf,"pull",4) == 0){
                            fds[i].events |= POLLOUT;
                        }
                        if(strncmp(requestP[fds[i].fd].buf,"rea",3) == 0){ // 處理要寫入的post               
                            char name[FROM_LEN + 1], content[CONTENT_LEN + 1];
                            int num = requestP[fds[i].fd].buf[4] - '0';
                            int index = 0;
                            memset(name, 0, sizeof(name));
                            for ( int ind = 5; ind < 10; ++ind){
                                if(requestP[fds[i].fd].buf[ind] != '\0') {
                                    name[index++] = requestP[fds[i].fd].buf[ind];
                                }
                                else {
                                    name[index] = '\0';
                                    break;
                                }
                            }
                            index = 0;
                            memset(content, 0, sizeof(content));
                            for ( int ind = 10; ind < 30; ++ind){
                                if(requestP[fds[i].fd].buf[ind] != '\0') {
                                    content[index++] = requestP[fds[i].fd].buf[ind];
                                }
                                else {
                                    content[index] = '\0';
                                    break;
                                }
                            }
                            pwrite(board_fd, name, 5, num * 25);
                            pwrite(board_fd, content, 20, num * 25 + 5);
                            fprintf(stderr,"Write in to record %d\n", num);
                            printf("[Log] Receive post from %s\n", name);
                            //fflush(stdout);
                            lock.l_start = num * 25;
                            lock.l_type = F_UNLCK;
                            fcntl(board_fd, F_SETLK, &lock);
                            writing[num] = 0;
                        }
                    }
                        

                }
            }
            if(fds[i].revents & POLLOUT){
                if(strncmp(requestP[fds[i].fd].buf,"pull",4) == 0){
                        pull_board(requestP[fds[i].fd].buf, board_fd, fds, i);
                    }
                if(strncmp(requestP[fds[i].fd].buf,"post",4) == 0){
                        //printf("in POLLOUT section %s\n",requestP[fds[i].fd].buf);
                        write(fds[i].fd, requestP[fds[i].fd].buf, BUFFER_SIZE);
                        fds[i].events &= ~POLLOUT;
                    }
                if(strncmp(requestP[fds[i].fd].buf,"failed",6) == 0){
                        write(fds[i].fd, requestP[fds[i].fd].buf, BUFFER_SIZE);
                        fds[i].events &= ~POLLOUT;
                    }
            }
        }
        // TODO: handle requests from clients
    
    }
    free(requestP);
    return 0;
}

// ======================================================================================================
// You don't need to know how the following codes are working



void pull_board(char *buf, int board_fd, struct pollfd *fds, int i){
    fprintf(stderr,"Call pull_board\n");
    ssize_t read_byte, write_byte = 0;
    char name[FROM_LEN + 1], content[CONTENT_LEN + 1], c[2];
    strcpy(buf, "");
    
    int lock_num = 0;
    for(int num = 0; num < 10; ++num){
        struct flock b_lock;
        b_lock.l_type = F_WRLCK;
        b_lock.l_whence = SEEK_SET; // 從文件開始
        b_lock.l_start = num * 25;        // 鎖定的起始位移
        b_lock.l_len = 25;
        if(fcntl(board_fd, F_GETLK, &b_lock) == -1) {
            fprintf(stderr,"GETLCK error at record %d\n", num);
            fprintf(stderr,"errno : %d\n", errno);
        }
        else{
            if(b_lock.l_type != F_UNLCK || writing[num]){
                fprintf(stderr,"record %d is being written\n", num);
                ++lock_num;
                continue;
            }
        }
        strcpy(name,"");
        strcpy(content,"");
        if(lseek(board_fd, num * 25 ,SEEK_SET) == -1) fprintf(stderr,"lseek error\n");
        else{
            for ( int now = 0; now < FROM_LEN; ++now){
                if ((read_byte = read(board_fd, c, 1)) < 0){
                    fprintf(stderr,"Reading board name error\n");                    
                }
                else{
                    if(c[0] == '\0') break;
                    strcat(name, c);
                }
            }
        }
        if ( strcmp(name, "") ==  0) break;
        if(lseek(board_fd, num * 25 + 5,SEEK_SET) == -1) fprintf(stderr,"lseek error\n");
        else{
            for ( int now = 0; now < CONTENT_LEN; ++now){
                if ((read_byte = read(board_fd, c, 1)) < 0){
                    fprintf(stderr,"Reading board content error\n");                    
                }
                else{
                    if(c[0] == '\0') {
                        strcat(content, c);
                        break;
                    }
                    strcat(content, c);
                }
            }
        }
        strcat(buf, "FROM: ");
        strcat(buf, name);
        strcat(buf, "\n");
        strcat(buf, "CONTENT:\n");
        strcat(buf, content);
        strcat(buf, "\n");
    }
    if(lock_num){
        printf("[Warning] Try to access locked post - %d\n", lock_num);
        //fflush(stdout);
    }
    write_byte = write(fds[i].fd, buf, BUFFER_SIZE);
    if(write_byte <= 0){
        fprintf(stderr,"Writing to client error\n");
        close(fds[i].fd);
    } 
    else{
        fds[i].events &= ~POLLOUT;
        fprintf(stderr,"close POLLOUT\n");
    }
}

static void init_request(request* reqP) {
    reqP->conn_fd = -1;
    reqP->buf_len = 0;
    reqP->id = 0;
}

static void free_request(request* reqP) {
    init_request(reqP);
}

static void init_server(unsigned short port) {
    struct sockaddr_in servaddr;
    int tmp;

    gethostname(svr.hostname, sizeof(svr.hostname));
    svr.port = port;

    svr.listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (svr.listen_fd < 0) ERR_EXIT("socket");

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);
    tmp = 1;
    if (setsockopt(svr.listen_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&tmp, sizeof(tmp)) < 0) {
        ERR_EXIT("setsockopt");
    }
    if (bind(svr.listen_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        ERR_EXIT("bind");
    }
    if (listen(svr.listen_fd, 1024) < 0) {
        ERR_EXIT("listen");
    }

    // Get file descripter table size and initialize request table
    maxfd = getdtablesize();
    requestP = (request*) malloc(sizeof(request) * maxfd);
    if (requestP == NULL) {
        ERR_EXIT("out of memory allocating all requests");
    }
    for (int i = 0; i < maxfd; i++) {
        init_request(&requestP[i]);
    }
    requestP[svr.listen_fd].conn_fd = svr.listen_fd;
    strcpy(requestP[svr.listen_fd].host, svr.hostname);

    return;
}
