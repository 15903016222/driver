#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#define GPIO_I2C_READ    0x100001
#define GPIO_I2C_WRITE   0x100002

struct eeprom_data {
    unsigned char addr;
    unsigned char data;
};

int main(void)
{
    struct eeprom_data eeprom;
    int i;
    char *pversion = "S14091207"; //软件版本信息
    char *p = pversion;
    char buf[10] = {0};

    int fd = open("/dev/at24c02", O_RDWR);
    if (fd < 0)
        return -1;

    //写入版本信息
    for (i = 0; i < 9; i++) {
        eeprom.data = *p++;
        eeprom.addr = i;
        ioctl(fd, GPIO_I2C_WRITE, &eeprom);
        usleep(5000);
    }
    
    //读取版本信息
    for (i = 0; i < 9; i++) {
        eeprom.addr = 0;
        ioctl(fd, GPIO_I2C_READ, &eeprom);
        buf[i] = eeprom.data;
    }

    //打印版本信息
    printf("version = %s\n", buf);
    close(fd);
    return 0;
}
