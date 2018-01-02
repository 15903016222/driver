#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//./btn_test r
//./btn_test w
int main(int argc, char *argv[])
{
    int fd;

    if (argc != 2) {
        printf("Usage: %s <r|w>\n", argv[0]);
        return -1;
    }   

    fd = open("/dev/mybtn", O_RDWR);
    if (fd < 0)
        return -1;

    if (!strcmp(argv[1], "r"))
        read(fd, NULL, 0); //启动读进程
    else if(!strcmp(argv[1], "w"))
        write(fd, NULL, 0); //启动写进程

    close(fd);
    return 0;
}







