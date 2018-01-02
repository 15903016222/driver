#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <asm/gpio.h>
#include <plat/gpio-cfg.h>
#include <linux/sched.h> //TASK_*
#include <linux/input.h> //KEY_* 标准按键值
#include <linux/uaccess.h>

//声明上报按键信息的数据结构
struct btn_event {
    int code;
    int state;
};

//声明描述按键信息的数据结构
struct btn_resource {
    char *name; //按键名
    int irq; //中断号
    int gpio; //GPIO编号
    int code; //按键值
};

//定义初始化按键对象
static struct btn_resource btn_info[] = {
    {
        .name = "KEY_UP",
        .irq = IRQ_EINT(0),
        .gpio = S5PV210_GPH0(0),
        .code = KEY_UP
    },
    {
        .name = "KEY_DOWN",
        .irq = IRQ_EINT(1),
        .gpio = S5PV210_GPH0(1),
        .code = KEY_DOWN
    }
};

//定义上报按键信息的对象
static struct btn_event g_data;

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
        copy_to_user(buf, &g_data, sizeof(g_data));
    }

    return count;
}

//中断处理函数
static irqreturn_t button_isr(int irq, void *dev)
{
    //1.通过dev获取对应的按键的硬件信息
    struct btn_resource *pdata = 
            (struct btn_resource *)dev;

    //2.获取按键的状态和键值
    g_data.state = gpio_get_value(pdata->gpio);
    g_data.code = pdata->code;

    //3.唤醒休眠的进程
    wake_up(&rwq);
    return IRQ_HANDLED;
}

//定义初始化硬件操作接口对象
static struct file_operations btn_fops = {
    .owner = THIS_MODULE,
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
    int i;
    //申请GPIO资源注册中断处理函数
    for (i = 0; i < ARRAY_SIZE(btn_info); i++) {
        gpio_request(btn_info[i].gpio, 
                        btn_info[i].name);
        request_irq(btn_info[i].irq, button_isr,
            IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,
                    btn_info[i].name,
                    &btn_info[i]);
    }
    //注册
    misc_register(&btn_misc);
    //初始化等待队列头
    init_waitqueue_head(&rwq);
    return 0;
}

static void btn_exit(void)
{
    int i;
    for (i = 0; i < ARRAY_SIZE(btn_info); i++) {
        gpio_free(btn_info[i].gpio); 
        free_irq(btn_info[i].irq, &btn_info[i]);
    }
    //卸载
    misc_deregister(&btn_misc);
}
module_init(btn_init);
module_exit(btn_exit);
MODULE_LICENSE("GPL");
