#include <linux/init.h>
#include <linux/module.h>
#include <asm/gpio.h>
#include <plat/gpio-cfg.h>

//声明描述LED硬件相关的数据结构
struct led_resource {
    int gpio; //记录GPIO的软件编号
    char *name; //GPIO的名称
};

//定义初始化两个LED的硬件信息对象
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

static int led_init(void)
{
    int i;
    //1.申请GPIO资源
    //2.配置GPIO为输出口,输出1，开灯
    for (i = 0; i < ARRAY_SIZE(led_info); i++) {
        gpio_request(led_info[i].gpio, 
                     led_info[i].name);
        gpio_direction_output(led_info[i].gpio, 1);
    }
    return 0;
}

static void led_exit(void)
{
    int i;
    //1.设置GPIO为0,关灯
    //2.释放GPIO资源
    for (i = 0; i < ARRAY_SIZE(led_info); i++) {
        gpio_set_value(led_info[i].gpio, 0);
        gpio_free(led_info[i].gpio); 
    }
}
module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
