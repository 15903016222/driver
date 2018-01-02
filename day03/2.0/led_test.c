#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//./led_test on 
//./led_test off

int main(int argc, char *argv[])
{
    int fd;
    int ucmd; //定义用户缓冲区

    if (argc != 2) {
        printf("用法:%s <on|off>\n", argv[0]);
        return -1;
    }

    //应用open->软中断->内核sys_open->驱动可以没有,返回
    //成功
    fd = open("/dev/myled", O_RDWR);
    if (fd < 0) {
        printf("打开设备失败!\n");
        return -1;
    }

    if (!strcmp(argv[1], "on")) 
        ucmd = 1;
    else if (!strcmp(argv[1], "off"))
        ucmd = 0;
    
    //应用write->软中断->内核sys_write->驱动led_write
    write(fd, &ucmd, 4);

    //应用close->软中断->内核sys_close->驱动可以没有
    close(fd);
    return 0;
}
