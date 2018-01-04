#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>

//定义初始化LED的硬件信息
static struct resource led_res[] = {
    //描述寄存器地址信息
    {
        .start = 0xE0200080,
        .end = 0xE0200080 + 8,
        .flags = IORESOURCE_MEM
    },
    //描述LED1的GPIO编号信息
    {
        .start = 3,
        .end = 3,
        .flags = IORESOURCE_IRQ
    },
    //描述LED2的GPIO编号信息
    {
        .start = 4,
        .end = 4,
        .flags = IORESOURCE_IRQ
    }
};

static void led_release(struct device *dev) {}

//定义初始化LED的硬件节点对象
static struct platform_device led_dev = {
    .name = "tarena", //将来用于匹配
    .id = -1,//标号
    .resource = led_res, //装载硬件信息 
    .num_resources = ARRAY_SIZE(led_res), 
    .dev = {
        .release = led_release //屏蔽警告而已
    } 
};

static int led_dev_init(void)
{
    //注册硬件节点
    platform_device_register(&led_dev);
    return 0;
}

static void led_dev_exit(void)
{
    //卸载硬件信息
    platform_device_unregister(&led_dev);
}
module_init(led_dev_init);
module_exit(led_dev_exit);
MODULE_LICENSE("GPL");
