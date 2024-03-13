#include <unistd.h>
#include <fcntl.h>
int main(){
	int fd = openat( AT_FDCWD, "../test", O_RDWR | O_CREAT, 0644);
	return 0;
}
