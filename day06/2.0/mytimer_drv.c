#include <linux/init.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <asm/gpio.h>
#include <plat/gpio-cfg.h>

//声明描述LED的硬件数据结构
//定义初始化2个LED的硬件信息对象

//定义定时器对象
static struct timer_list mytimer;

//超时处理函数
//基于软中断实现,不能进行休眠操作
static void mytimer_function(unsigned long data)
{
    //0.开关灯,如果开那就关,如果关那就开
    printk("%s: g_data = %#x\n",
                __func__, *(int *)data);

    //1.如果要重复执行超时处理函数,需要再次启动定时器
    mod_timer(&mytimer, 
            jiffies + msecs_to_jiffies(5000));
}

static int g_data = 0x5555;

static int mytimer_init(void)
{
    //0.申请GPIO资源,配置为输出口,输出0
    //1.初始化定时器对象
    init_timer(&mytimer);
    //2.设置定时器的超时时间
    mytimer.expires = jiffies + msecs_to_jiffies(5000);
    //3.设置定时器的超时处理函数
    mytimer.function = mytimer_function;
    //4.给超时处理函数传递参数
    mytimer.data = (unsigned long)&g_data;

    //5.向内核添加注册定时器对象,内核开始倒计时5.4...0
    add_timer(&mytimer);
    printk("定时器添加完毕！\n");
    return 0;
}

static void mytimer_exit(void)
{
    //1.释放GPIO资源
    //2.删除定时器对象
    del_timer(&mytimer);
}
module_init(mytimer_init);
module_exit(mytimer_exit);
MODULE_LICENSE("GPL");
