#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>
#define ERR_EXIT(s) perror(s), exit(errno);
int main(int argc, char *argv[]){
    int fdin, fdout;
    void *src, *dst;
    struct stat statbuf;

    if( argc != 3 ) ERR_EXIT("usage: ./a.out <src> <dst>  , it copy <src> to <dst>");

    if(( fdin = open(argv[1], O_RDONLY) ) < 0) ERR_EXIT("can't open src");

    if(( fdout = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, 0644) ) < 0) ERR_EXIT("can't open dst");

    if( fstat(fdin, &statbuf) < 0 ) ERR_EXIT("fstat");

    if( lseek(fdout, statbuf.st_size - 1, SEEK_SET) == -1 ) ERR_EXIT("lseek");

    if( write( fdout, "", 1) < 0 ) ERR_EXIT("write");

    if( (src = mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, fdin, 0)) == MAP_FAILED) ERR_EXIT("mmap fdin");


    if( (dst = mmap(0, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fdout, 0)) == MAP_FAILED) ERR_EXIT("mmap fdout");

    memcpy(dst, src, statbuf.st_size);
    exit(0);
}