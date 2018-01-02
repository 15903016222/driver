#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/io.h>

static unsigned long *gpiocon, *gpiodata;
static int pin1, pin2;

#define LED_ON  0x100001
#define LED_OFF 0x100002

static long led_ioctl(struct file *file,
                        unsigned int cmd,
                        unsigned long arg)
{
    //1.定义内核缓冲区
    int kindex;

    //2.拷贝用户缓冲区到内核
    copy_from_user(&kindex, (int *)arg, 4);

    //3.解析命令,操作硬件
    switch(cmd) {
        case LED_ON:
            if(kindex == 1) 
                *gpiodata |= (1 << pin1);
            else if(kindex == 2)
                *gpiodata |= (1 << pin2);
            break;
        case LED_OFF:
            if(kindex == 1) 
                *gpiodata &= ~(1 << pin1);
            else if(kindex == 2)
                *gpiodata &= ~(1 << pin2);
            break;
    }
    //调试信息
    printk("%s:配置寄存器=%#x, 数据寄存器=%#x\n",
            __func__, *gpiocon, *gpiodata);
    return 0;
}

static struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = led_ioctl
};

static struct miscdevice led_misc = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "myled",
    .fops = &led_fops
};

//pdev指向led_dev.c的led_dev对象
static int led_probe(struct platform_device *pdev)
{
    struct resource *reg_res;
    struct resource *pin1_res;
    struct resource *pin2_res;

    //1.通过pdev获取硬件信息
    //reg_res = &led_res[0];
    reg_res = platform_get_resource(pdev, 
                            IORESOURCE_MEM, 0);
    pin1_res = platform_get_resource(pdev,
                            IORESOURCE_IRQ, 0);
    pin2_res = platform_get_resource(pdev,
                            IORESOURCE_IRQ, 1);
    
    //2.处理硬件信息
    //2.1该映射的映射
    gpiocon = ioremap(reg_res->start, 
                        reg_res->end-reg_res->start);        
    gpiodata = gpiocon + 1;
    
    pin1 = pin1_res->start; //3
    pin2 = pin2_res->start; //4

    //2.2配置为输出口,输出0
    *gpiocon &= ~((0xf << (pin1*4)) 
                | (0xf << (pin2*4)));
    
    *gpiocon |= ((1 << (pin1*4)) | (1 << (pin2*4)));
    *gpiodata &= ~((1 << pin1) | (1 << pin2));
    
    //2.3该申请的申请
    //2.4该注册的注册
    //3.注册混杂设备驱动
    misc_register(&led_misc);
    return 0;
}

//pdev指向led_dev.c的led_dev对象
static int led_remove(struct platform_device *pdev)
{
    //跟probe对着干！
    misc_deregister(&led_misc);
    *gpiodata &= ~((1 << pin1) | (1 << pin2));
    iounmap(gpiocon);
    return 0;
}

//定义初始化LED的软件节点对象
static struct platform_driver led_drv = {
    .driver = {
        .name = "tarena" //用于匹配
    },
    .probe = led_probe, //匹配成功,内核调用
    .remove = led_remove //删除硬件或软件节点,内核调用
};

static int led_drv_init(void)
{
    //注册软件节点
    platform_driver_register(&led_drv);
    return 0;
}

static void led_drv_exit(void)
{
    //卸载软件信息
    platform_driver_unregister(&led_drv);
}
module_init(led_drv_init);
module_exit(led_drv_exit);
MODULE_LICENSE("GPL");
