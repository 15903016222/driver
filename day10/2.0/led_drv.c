#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>

//pdev指向led_dev.c的led_dev对象
static int led_probe(struct platform_device *pdev)
{
    printk("%s\n", __func__);
    //1.通过pdev获取硬件信息
    //2.处理硬件信息
    //2.1该映射的映射
    //2.2该初始化的初始化
    //2.3该申请的申请
    //2.4该注册的注册
    //3.注册混杂设备驱动
    return 0;
}

//pdev指向led_dev.c的led_dev对象
static int led_remove(struct platform_device *pdev)
{
    printk("%s\n", __func__);
    //跟probe对着干！
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
