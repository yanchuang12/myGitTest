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
        close(fd);
    }
    return 0;
}

