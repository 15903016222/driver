#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <asm/gpio.h>
#include <plat/gpio-cfg.h>

//声明描述LED硬件信息的数据结构
struct led_resource {
    int gpio;
    char *name;
};

//定义初始化2个LED的硬件信息对象
static struct led_resource led_info[] = {
    {
        .gpio = S5PV210_GPC1(3),
        .name = "LED1"
    },
    {
        .gpio = S5PV210_GPC1(4),
        .name = "LED2"
    }
};

#define LED_ON  0x100001
#define LED_OFF 0x100002

//int uindex=1,ioctl(fd, LED_ON, &uindex);
static long led_ioctl(struct file *file,
                        unsigned int cmd,
                        unsigned long arg)
{
    //1.分配内核缓冲区
    int kindex;

    //2.拷贝用户缓冲区数据到内核缓冲区
    copy_from_user(&kindex, (int *)arg, 4);

    //3.解析命令,操作硬件
    switch(cmd) {
        case LED_ON:
            gpio_set_value(led_info[kindex-1].gpio, 1);
            break;
        case LED_OFF:
            gpio_set_value(led_info[kindex-1].gpio, 1);
            break;
    }
    return 0;
}
//定义初始化硬件操作接口对象
static struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = led_ioctl //控制接口
};

//定义初始化混杂设备对象
static struct miscdevice led_misc = {
    .minor  = MISC_DYNAMIC_MINOR, //让内核帮你分配一个次设备号
    .name   = "led", //自动创建设备文件/dev/myled
    .fops   = &led_fops, //添加硬件操作接口
};

static int led_init(void)
{
    int i;

    //1.申请GPIO资源
    //2.配置为输出，输出0
    for (i = 0; i < ARRAY_SIZE(led_info); i++) {
        gpio_request(led_info[i].gpio, 
                        led_info[i].name);
        gpio_direction_output(led_info[i].gpio, 0);
    }
    //3.注册混杂设备对象到内核
    misc_register(&led_misc);
    return 0;
}

static void led_exit(void)
{
    int i;
    //1.卸载混杂设备对象
    misc_deregister(&led_misc);
    //2.释放GPIO资源
    for (i = 0; i < ARRAY_SIZE(led_info); i++) {
        gpio_direction_output(led_info[i].gpio, 0);
        gpio_free(led_info[i].gpio);
    }
}
module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");






