#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/io.h> //ioremap
#include <linux/uaccess.h>

//定义保存寄存器物理地址对应的内核虚拟地址指针变量
static unsigned long *gpiocon, *gpiodata;

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
                *gpiodata |= (1 << 3);
            else if(kindex == 2)
                *gpiodata |= (1 << 4);
            break;
        case LED_OFF:
            if(kindex == 1) 
                *gpiodata &= ~(1 << 3);
            else if(kindex == 2)
                *gpiodata &= ~(1 << 4);
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

static int led_init(void)
{
    //将寄存器的物理地址映射到内核虚拟地址上
    gpiocon = ioremap(0xE0200080, 8);
    gpiodata = gpiocon + 1;

    //配置GPIO为出口,输出0
    *gpiocon &= ~((0xf << 12) | (0xf << 16));
    *gpiocon |= ((1 << 12) | (1 << 16));
    *gpiodata &= ~((1 << 3) | (1 << 4));

    misc_register(&led_misc);
    return 0;
}

static void led_exit(void)
{
    //输出0
    *gpiodata &= ~((1 << 3) | (1 << 4));
    //解除地址映射
    iounmap(gpiocon);
    misc_deregister(&led_misc);
}
module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
