#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#define LED_ON  0x100001
#define LED_OFF 0x100002

int main(int argc, char *argv[])
{
    int fd1;
    int fd2;

    fd1 = open("/dev/myled1", O_RDWR);
    if (fd1 < 0)
        return -1;
    
    fd2 = open("/dev/myled2", O_RDWR);
    if (fd2 < 0)
        return -1;
   
    while(1) {
        //开第一个灯
        ioctl(fd1, LED_ON);
        sleep(1);
        //开第二个灯
        ioctl(fd2, LED_ON);
        sleep(1);
        //关第一个灯
        ioctl(fd1, LED_OFF);
        sleep(1);
        //关第二个灯
        ioctl(fd2, LED_OFF);
        sleep(1);
    }
    
    close(fd1);
    close(fd2);
    return 0;
}





