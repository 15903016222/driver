#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <asm/gpio.h>
#include <plat/gpio-cfg.h>
#include <linux/device.h> //自动创建设备文件

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

    //1.通过file指针获取inode
    //通过inode获取次设备号
    //应用访问myled1,minor=0;访问myled2,minor=1
    struct inode *inode = file->f_path.dentry->d_inode;
    int minor = MINOR(inode->i_rdev);

    //2.解析用户控制命令然后操作硬件
    switch(cmd) {
        case LED_ON:
            gpio_set_value(led_info[minor].gpio, 1);
            printk("%s: 开第%d个灯\n", 
                    __func__, minor+1);
            break;
        case LED_OFF:
            gpio_set_value(led_info[minor].gpio, 0);
            printk("%s: 关第%d个灯\n", 
                    __func__, minor+1);
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

//定义设备类指针
static struct class *cls; 

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
    alloc_chrdev_region(&dev, 0, 2, "tarena");
    //4.初始化字符设备对象
    cdev_init(&led_cdev, &led_fops);
    //5.注册字符设备对象到内核
    cdev_add(&led_cdev, dev, 2);
    //6.定义一个设备类对象，类似长树枝
    cls = class_create(THIS_MODULE, "tarena");
    //7.创建设备文件/dev/myled1,类似长苹果
    device_create(cls, NULL, 
                MKDEV(MAJOR(dev),0), NULL, "myled1");
    //8.创建设备文件/dev/myled2,类似长苹果
    device_create(cls, NULL, 
                MKDEV(MAJOR(dev),1), NULL, "myled2");
    return 0;
}

static void led_exit(void)
{
    int i;
    //0.删除设备文件和设备类
    device_destroy(cls, MKDEV(MAJOR(dev),0)); //摘苹果
    device_destroy(cls, MKDEV(MAJOR(dev),1)); //摘苹果
    class_destroy(cls); //砍树枝

    //1.卸载字符设备对象
    cdev_del(&led_cdev);
    //2.释放设备号
    unregister_chrdev_region(dev, 2);
    //3.释放GPIO资源
    for (i = 0; i < ARRAY_SIZE(led_info); i++) {
        gpio_direction_output(led_info[i].gpio, 0);
        gpio_free(led_info[i].gpio);
    }
}
module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
