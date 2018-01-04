#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
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

//定义设备操作控制命令
#define LED_ON  (0x100001)
#define LED_OFF (0x100002)

//应用ioctl->软中断->内核sys_ioctl->驱动led_ioctl
//应用：int uindex=1;ioctl(fd, LED_ON, &uindex);
static long led_ioctl(struct file *file,
                        unsigned int cmd,
                        unsigned long arg)
{
    //1.由于应用传递第三个参数用户缓冲区的首地址,
    //所以分配内核缓冲区
    int kindex;

    //2.拷贝用户缓冲区的数据到内核缓冲区
    copy_from_user(&kindex, (int *)arg, 4);

    //3.解析用户控制命令然后操作硬件
    switch(cmd) {
        case LED_ON:
            gpio_set_value(led_info[kindex-1].gpio, 1);
            printk("%s: 开第%d个灯\n", 
                    __func__, kindex);
            break;
        case LED_OFF:
            gpio_set_value(led_info[kindex-1].gpio, 0);
            printk("%s: 关第%d个灯\n", 
                    __func__, kindex);
            break;
        default:
            printk("无效的命令\n");
            return -1;
    }
    return 0;
}

//定义初始化硬件操作接口对象
static struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = led_ioctl //控制接口
};

//定义字符设备对象和设备号对象
static struct cdev led_cdev;
static dev_t dev;

static int led_init(void)
{
    int i;
    //1.申请GPIO资
    //2.配置GPIO为输出口，输出0
    for (i = 0; i < ARRAY_SIZE(led_info); i++) {
        gpio_request(led_info[i].gpio, 
                        led_info[i].name);
        gpio_direction_output(led_info[i].gpio, 0);
    }
    //3.申请设备号
    alloc_chrdev_region(&dev, 0, 1, "tarena");
    //4.初始化字符设备对象
    cdev_init(&led_cdev, &led_fops);
    //5.注册字符设备对象到内核
    cdev_add(&led_cdev, dev, 1);
    return 0;
}

static void led_exit(void)
{
    int i;
    //1.卸载字符设备对象
    cdev_del(&led_cdev);
    //2.释放设备号
    unregister_chrdev_region(dev, 1);
    //3.释放GPIO资源
    for (i = 0; i < ARRAY_SIZE(led_info); i++) {
        gpio_direction_output(led_info[i].gpio, 0);
        gpio_free(led_info[i].gpio);
    }
}
module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
