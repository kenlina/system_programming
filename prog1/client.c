#include "hw1.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>
#define ERR_EXIT(a) do { perror(a); exit(1); } while(0)
#define BUFFER_SIZE 512

typedef struct {
    char* ip; // server's ip
    unsigned short port; // server's port
    int conn_fd; // fd to talk with server
    char buf[BUFFER_SIZE]; // data sent by/to server
    size_t buf_len; // bytes used by buf
} client;

client cli;
static void init_client(char** argv);

int main(int argc, char** argv){
    
    // Parse args.
    if(argc!=3){
        ERR_EXIT("usage: [ip] [port]");
    }

    // Handling connection
    init_client(argv);
    fprintf(stderr, "connect to %s %d\n", cli.ip, cli.port);
    printf("==============================\n" 
           "Welcome to CSIE Bulletin board\n"
           "==============================\n");
    struct pollfd fds[1];
    nfds_t nfds= 1;
    fds[0].fd = cli.conn_fd;
    fds[0].events = POLLOUT;
    int ret = poll(fds, nfds, -1);
    if (ret < 0) ERR_EXIT("poll");
    int write_byte, read_byte;

    if(fds[0].revents & POLLOUT){
            strcpy(cli.buf, "pull");
            if((write_byte = write(cli.conn_fd, cli.buf, strlen(cli.buf))) == -1)
                ERR_EXIT("Write error");
            fds[0].events &= ~POLLOUT;
            fds[0].events |= POLLIN;
            ret = poll(fds, nfds, -1);
            if (ret < 0) ERR_EXIT("poll");
            if(fds[0].revents & POLLIN){
                if((read_byte = read(cli.conn_fd, cli.buf, BUFFER_SIZE)) == -1)
                    ERR_EXIT("read");
                printf("%s", cli.buf);
                fds[0].events &= ~POLLIN;
            }
    }
    printf("==============================\n");
    while(1){
        // TODO: handle user's input
        printf("Please enter your command (post/pull/exit): ");
        fgets(cli.buf, BUFFER_SIZE, stdin);
        fds[0].events = POLLOUT;
        ret = poll(fds, nfds, -1);
        if (ret < 0) ERR_EXIT("poll");
        

        if(fds[0].revents & POLLOUT){
            if((write_byte = write(cli.conn_fd, cli.buf, strlen(cli.buf))) == -1)
                ERR_EXIT("Write error");
            fds[0].events &= ~POLLOUT;
        }
        if(strncmp(cli.buf,"post", 4) == 0){
            fds[0].events |= POLLIN;
            ret = poll(fds, nfds, -1);
            if (ret < 0) ERR_EXIT("poll");
            if(fds[0].revents & POLLIN){
                if((read_byte = read(cli.conn_fd, cli.buf, BUFFER_SIZE)) == -1)
                    ERR_EXIT("read");
                if(strncmp(cli.buf,"post",4) == 0){
                    char num = cli.buf[4];
                    char name[7], content[22];
                    printf("FROM: ");
                    fgets(name, sizeof(name), stdin);
                    char *newline = strchr(name, '\n');
                    if (newline) {
                        *newline = '\0'; // 替換換行符為空字符
                    }
                    printf("CONTENT:\n");
                    fgets(content, sizeof(content), stdin);
                    newline = strchr(content, '\n');
                    if (newline) {
                        *newline = '\0'; // 替換換行符為空字符
                    }
                    strcpy(cli.buf, "");
                    strncpy(cli.buf, "rea", 4);
                    strncpy(cli.buf + 4, &num, 1);
                    strncpy(cli.buf + 5, name, 5);
                    strncpy(cli.buf + 10, content, 20);
                    
                    fds[0].events = POLLOUT;
                    ret = poll(fds, nfds, -1);
                    if (ret < 0) ERR_EXIT("poll");
                    if(fds[0].revents & POLLOUT){
                        if((write_byte = write(cli.conn_fd, cli.buf, BUFFER_SIZE)) == -1)
                            ERR_EXIT("Write error");
                        fds[0].events &= ~POLLOUT;
                    }
                }
                else{
                    printf("[Error] Maximum posting limit exceeded\n");
                }
                fds[0].events &= ~POLLIN;
            }
        }
        if(strncmp(cli.buf,"pull", 4) == 0){
            fds[0].events |= POLLIN;
            ret = poll(fds, nfds, -1);
            if (ret < 0) ERR_EXIT("poll");
            if(fds[0].revents & POLLIN){
                if((read_byte = read(cli.conn_fd, cli.buf, BUFFER_SIZE)) == -1)
                    ERR_EXIT("read");
                printf("==============================\n");
                printf("%s", cli.buf);
                printf("==============================\n");
                fds[0].events &= ~POLLIN;
            }
        }
        if(strncmp(cli.buf,"exit", 4) == 0){
            close(cli.conn_fd);
            fprintf(stderr,"Exit client process\n");
            return 0;
        }
        
    }
 
}

static void init_client(char** argv){
    
    cli.ip = argv[1];

    if(atoi(argv[2])==0 || atoi(argv[2])>65536){
        ERR_EXIT("Invalid port");
    }
    cli.port=(unsigned short)atoi(argv[2]);

    struct sockaddr_in servaddr;
    cli.conn_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(cli.conn_fd<0){
        ERR_EXIT("socket");
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(cli.port);

    if(inet_pton(AF_INET, cli.ip, &servaddr.sin_addr)<=0){
        ERR_EXIT("Invalid IP");
    }

    if(connect(cli.conn_fd, (struct sockaddr*)&servaddr, sizeof(servaddr))<0){
        ERR_EXIT("connect");
    }

    return;
}
