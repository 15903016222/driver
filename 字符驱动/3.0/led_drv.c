#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <asm/gpio.h>
#include <plat/gpio-cfg.h>
#include <linux/uaccess.h> //copy_from_user

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

//应用write->软中断->内核sys_write->驱动led_write
//应用：int ucmd = 1;write(fd, &ucmd, 4); //开
//应用：int ucmd = 0;write(fd, &ucmd, 4); //关
static ssize_t led_write(struct file *file,
                        const char __user *buf,
                        size_t count,
                        loff_t *ppos)
{
    //1.分配内核缓冲区
    int kcmd;
    int i;

    //2.拷贝用户缓冲区的数据到内核缓冲区
    //注意：千万不能直接从用户缓冲区读取数据
    //buf保存的ucmd的首地址
    //kcmd = *(int *)buf; //相当危险
    copy_from_user(&kcmd, buf, 4);
    
    //3.将数据最终写入到硬件数据寄存器
    for (i = 0; i < ARRAY_SIZE(led_info); i++)
        gpio_set_value(led_info[i].gpio, kcmd);

    printk("%s\n", __func__);
    return count; //返回实际写入的字节数
}

//应用read->软中断->内核sys_read->驱动led_read
//应用：int ustate; read(fd, &ustate, 4);
//printf("%s\n", ustate?"开灯":"关灯");
static ssize_t led_read(struct file *file,
                        char __user *buf,
                        size_t count,
                        loff_t *ppos)
{
    //1.分配内核缓冲区
    int kstate;

    //2.读取硬件寄存器获取灯的开关状态保存在内核缓冲区
    kstate = gpio_get_value(led_info[0].gpio);

    //3.将内核缓冲区的数据拷贝到用户缓冲区
    //buf保存的用户缓冲区ustate的首地址
    //*(int *)buf = kstate //相当危险
    copy_to_user(buf, &kstate, 4);
    
    printk("%s\n", __func__);
    return count; //返回实际读取的字节数
}

//定义初始化LED的硬件操作接口对象
static struct file_operations led_fops = {
    .owner = THIS_MODULE,
    //open,release可以不用初始化
    //应用open,close永远返回成功
    //.open = led_open,
    //.release = led_close
    .write = led_write, //向设备写入数据
    .read = led_read //从设备读取数据
};

//定义字符设备对象
static struct cdev led_cdev;

//定义设备号对象
static dev_t dev;

static int led_init(void)
{
    int i;

    //1.申请GPIO资源
    //2.配置GPIO为输出,输出0
    for (i = 0; i < ARRAY_SIZE(led_info); i++) {
        gpio_request(led_info[i].gpio, 
                        led_info[i].name);
        gpio_direction_output(led_info[i].gpio, 0);
    }
    
    //3.申请设备号
    alloc_chrdev_region(&dev, 0, 1, "tarena");
    
    //4.初始化字符设备对象,添加硬件操作接口
    //led_cdev.ops = &led_fops
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
    //3.输出0，设备GPIO资源
    for (i = 0; i < ARRAY_SIZE(led_info); i++) {
        gpio_direction_output(led_info[i].gpio, 0);
        gpio_free(led_info[i].gpio);
    }
}
module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
