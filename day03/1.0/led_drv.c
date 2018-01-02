#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h> //struct cdev
#include <linux/fs.h> //struct file_operations
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
        .gpio = S5PV210_GPC0(3),
        .name = "LED1"
    },
    {
        .gpio = S5PV210_GPC0(4),
        .name = "LED2"
    }
};

//应用open->软中断->内核sys_open->驱动led_open
static int led_open(struct inode * inode,
                        struct file *file)
{
    int i;

    //1.开灯
    for (i = 0; i < ARRAY_SIZE(led_info); i++)
        gpio_set_value(led_info[i].gpio, 1);
    
    printk("%s\n", __func__);
    return 0; //成功返回0,失败返回负值
}

//应用close->软中断->内核sys_close->驱动led_close
static int led_close(struct inode * inode,
                        struct file *file)
{
    int i;

    //1.关灯
    for (i = 0; i < ARRAY_SIZE(led_info); i++)
        gpio_set_value(led_info[i].gpio, 0);
    
    printk("%s\n", __func__);
    return 0; //成功返回0,失败返回负值
}

//定义初始化LED的硬件操作接口对象
static struct file_operations led_fops = {
    .owner = THIS_MODULE, //死记
    .open = led_open, //打开设备接口
    .release = led_close //关闭设备接口
};

//定义LED的字符设备对象
static struct cdev led_cdev;

//定义设备号对象
static dev_t dev;

//insmod 
static int led_init(void)
{
    int i;

    //1.申请GPIO资源
    //2.配置GPIO为输出口,输出0(省电)
    for (i = 0; i < ARRAY_SIZE(led_info); i++) {
        gpio_request(led_info[i].gpio, 
                        led_info[i].name);
        gpio_direction_output(led_info[i].gpio, 0);
    }
    
    //3.申请设备号
    alloc_chrdev_region(&dev, 0, 1, "tarena");

    //4.初始化字符设备对象
    //给字符设备对象添加硬件操作接口
    cdev_init(&led_cdev, &led_fops);

    //5.向内核注册字符设别对象,一个真实字符设备驱动诞生
    cdev_add(&led_cdev, dev, 1);
    return 0;
}

//rmmod
static void led_exit(void)
{
    int i;
    //1.卸载字符设备对象
    cdev_del(&led_cdev);

    //2.释放设备号
    unregister_chrdev_region(dev, 1);

    //3.输出0(省电)
    //4.释放GPIO资源
    for (i = 0; i < ARRAY_SIZE(led_info); i++) {
        gpio_direction_output(led_info[i].gpio, 0);
        gpio_free(led_info[i].gpio);
    }
}
module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
