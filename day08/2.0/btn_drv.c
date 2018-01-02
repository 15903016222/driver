#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <asm/gpio.h>
#include <plat/gpio-cfg.h>
#include <linux/sched.h> //TASK_*

//定义初始化等待队列头对象(构造鸡妈妈)
static wait_queue_head_t rwq; //每一个节点存放读进程

//进程read->软中断->内核sys_read->驱动btn_read
static ssize_t btn_read(struct file *file,
                        char __user *buf,
                        size_t count,
                        loff_t *ppos)
{
    //1.定义初始化装载休眠进程的容器(构造小鸡)
    //将当前进程添加到容器中
    wait_queue_t wait;
    init_waitqueue_entry(&wait, current);

    //2.将当前进程添加到等待队列中
    add_wait_queue(&rwq, &wait);

    //3.设置当前进程的休眠状态为可中断
    set_current_state(TASK_INTERRUPTIBLE);

    printk("读进程[%s][%d]将进入休眠状态.!\n",
                    current->comm, current->pid);
    //4.当前进程进入真正的休眠状态
    schedule(); //停止不动,等待被唤醒
   
    //5.一旦被唤醒,设置进程的状态为运行
    //并且从等待队列中移除
    set_current_state(TASK_RUNNING);
    remove_wait_queue(&rwq, &wait); 
   
    //6.判断唤醒的原因
    if (signal_pending(current)) {
        printk("读进程[%s][%d]由于接收到了信号引起的唤醒!\n", current->comm, current->pid);
        return -ERESTARTSYS;
    } else {
        printk("读进程[%s][%d]由于写进程引起唤醒\n",
                current->comm, current->pid);
    }
    return count;
}

static ssize_t btn_write(struct file *file,
                        const char __user *buf,
                        size_t count,
                        loff_t *ppos)
{
    //唤醒读进程
    printk("写进程[%s][%d]唤醒读进程!\n",
                current->comm, current->pid);
    wake_up_interruptible(&rwq);
    return count;
}

//定义初始化硬件操作接口对象
static struct file_operations btn_fops = {
    .owner = THIS_MODULE,
    .write = btn_write, //写
    .read = btn_read //读
};

//定义初始化混杂设备对象
static struct miscdevice btn_misc = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "mybtn",
    .fops = &btn_fops
};

static int btn_init(void)
{
    //注册
    misc_register(&btn_misc);
    //初始化等待队列头
    init_waitqueue_head(&rwq);
    return 0;
}

static void btn_exit(void)
{
    //卸载
    misc_deregister(&btn_misc);
}
module_init(btn_init);
module_exit(btn_exit);
MODULE_LICENSE("GPL");
