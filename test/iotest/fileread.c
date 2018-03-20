#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

int main(int argc, char * argv[])
{
    if (argc < 2) {
        printf("please input file name\r\n");
        exit(1);
    }
    else {
        umask(0000);
        int fd = open(argv[1], O_RDWR|O_CREAT, 0666);
        if (fd < 0) {
            printf("open error\r\n");
            exit(1);
        }
        printf("success\r\n");
	char buf[128];
	memset(buf, 0, 128);
	int r = read(fd, buf, 128);
	if (r < 0) {
		printf("error\r\n");
	}
	else
		printf("buf = %s\r\n", buf);
        close(fd);
    }
    return 0;
}

