#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>

static int open_cnt = 1; //记录打开的次数,共享资源
static spinlock_t lock; //定义自旋锁对象

static int led_open(struct inode *inode,
                        struct file *file)
{
    unsigned long flags;

    //屏蔽中断,获取自旋锁
    spin_lock_irqsave(&lock, flags);

    //临界区
    if (--open_cnt != 0) {
        printk("设备已被打开!\n");
        open_cnt++;
        //释放锁,恢复中断
        spin_unlock_irqrestore(&lock,flags);
        return -EBUSY;
    }
    
    //释放锁,恢复中断
    spin_unlock_irqrestore(&lock,flags);
    printk("设备打开成功!\n");
    return 0;
}

static int led_close(struct inode *inode,
                        struct file *file)
{
    unsigned long flags;

    //屏蔽中断,获取自旋锁
    spin_lock_irqsave(&lock, flags);
    
    //临界区
    open_cnt++;

    //释放锁,恢复中断
    spin_unlock_irqrestore(&lock,flags);
    printk("设备关闭!\n");
    return 0;
}

//定义初始化硬件操作接口对象
static struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .release = led_close
};

//定义初始化混杂设备对象
static struct miscdevice led_misc = {
    .minor = MISC_DYNAMIC_MINOR, //自动分配一个次设备号
    .name = "myled", //自动创建设备文件/dev/myled
    .fops = &led_fops //添加操作接口
};

static int led_init(void)
{
    //1.注册混杂设备对象到内核
    misc_register(&led_misc);

    //2.初始化自旋锁
    spin_lock_init(&lock);
    return 0;
}

static void led_exit(void)
{
    //1.卸载混杂设备对象
    misc_deregister(&led_misc);
}
module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
