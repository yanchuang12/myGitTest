#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
int main()
{
	int fd = -1;
	fd = open("zhidao2.dat", O_CREAT|O_TRUNC|O_RDWR, 0666);
	if(fd < 0){
		perror("open");
		return -1;
	}
	char buf[64];
	strcpy(buf, "hello,i am coming");
	int count = strlen(buf);
	if(write(fd, buf, count) < 0){
		perror("wriet");
		return -1;
	}
	if(lseek(fd, 0, SEEK_SET) < 0){
		perror("read");
		return -1;
	}
	if(read(fd, buf, 10) < 0){
		perror("read");
		return -1;
	}
	buf[10] = 0x00;
	printf("%s\n", buf);
	if(fd > 0){
		close(fd);
		fd = -1;
	}
	return 0;
}
