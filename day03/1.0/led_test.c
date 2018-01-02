#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(void)
{
    int fd;

    //应用open->软中断->内核sys_open->驱动led_open
    fd = open("/dev/myled", O_RDWR);
    if (fd < 0) {
        printf("打开设备失败!\n");
        return -1;
    }

    sleep(3);

    //应用close->软中断->内核sys_close->驱动led_close
    close(fd);
    return 0;
}
