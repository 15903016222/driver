#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

int main(void)
{
    int fd;

    fd = open("/dev/myled", O_RDWR);
    if (fd < 0) {
        printf("打开设备失败!\n");
        return -1;
    }

    sleep(100000);

    close(fd);
    return 0;
}





