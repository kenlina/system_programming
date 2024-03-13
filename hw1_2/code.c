#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
ssize_t write_to_fd ( int fd, void *buf, size_t nbytes, off_t offset ){
    if ( lseek( fd, offset, SEEK_SET) < 0 ) return -1;
    return (write( fd, buf, nbytes) );
}
ssize_t write_to_fn ( char filename, void *buf, size_t nbytes, off_t offset ){
    int fd;
    ssize_t retval;
    if ( ( fd = open( filename, O_WRONLY ) )<0 ) return -1;
    if ( lseek( fd, offset, SEEK_SET) < 0 ) return -1;
    retval = write( fd, buf, nbytes );
    close( fd );
    return retval;
}