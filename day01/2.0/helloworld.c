#include <linux/init.h>
#include <linux/module.h>

//定义全局变量
static int irq;
static char *pstring;

//显式进行传参声明
module_param(irq, int, 0664);
module_param(pstring, charp, 0);

static int helloworld_init(void)
{
    printk("%s: irq = %d, pstring = %s\n",
                    __func__, irq, pstring);
    return 0;
}

static void helloworld_exit(void)
{
    printk("%s: irq = %d, pstring = %s\n",
                    __func__, irq, pstring);
}

module_init(helloworld_init);
module_exit(helloworld_exit);
MODULE_LICENSE("GPL");
