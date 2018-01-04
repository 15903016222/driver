#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//声明上报按键信息的数据结构
struct btn_event {
    int code;
    int state;
};

//./btn_test r
//./btn_test w
int main(int argc, char *argv[])
{
    int fd;
    struct btn_event btn;

    fd = open("/dev/mybtn", O_RDWR);
    if (fd < 0)
        return -1;

    while(1) {
        read(fd, &btn, sizeof(btn));
        printf("按键状态为%s,按键值为%d\n",
                btn.state ?"松开":"按下",
                btn.code);
    }
    close(fd);
    return 0;
}







