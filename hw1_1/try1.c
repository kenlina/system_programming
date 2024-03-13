#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
int  main(int  argc,  char  *argv[])
{
    int  fd1,  fd2;
    fd1 =  open  ("infile",O_RDONLY  );
    fd2  =  open  ("outfile",  O_WRONLY  |  O_CREAT,  0666);

    // do the redirections here
    dup2(fd1, 0);
    dup2(1, 2);
    dup2(fd2, 1);
    ftruncate(1, 0);

    execlp("./a.out",  "./a.out",  (char  *)0);
    return      0;
}