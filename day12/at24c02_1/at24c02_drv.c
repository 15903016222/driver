#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>

static struct i2c_device_id at24c02_id[] = {
        {"at24c02", 0}, 
        //”at24c02“必须和i2c_board_info的type一致,匹配靠它进行
};

//client指向内核帮咱们通过i2c_board_info实例化的i2c_client
//client里面包含设备地址addr
static int at24c02_probe(
            struct i2c_client *client, 
            struct i2c_device_id *id)
{
    printk("%s:addr = %#x\n", 
		__func__, client->addr);
    return 0; //成功返回0，失败返回负值
}

static int at24c02_remove(struct i2c_client *client) 
{
        printk("%s:addr = %#x\n", 
		__func__, client->addr);
    return 0; //成功返回0，失败返回负值
}

//分配初始化i2c_driver软件信息
static struct i2c_driver at24c02_drv = {
    .driver = {
        .name = "tarena" //不重要，匹配不靠它
    },
    .probe = at24c02_probe, //匹配成功执行
    .remove = at24c02_remove,
    .id_table = at24c02_id
};

static int at24c02_init(void)
{
    //注册i2c_driver
    i2c_add_driver(&at24c02_drv);
    return 0;
}

static void at24c02_exit(void)
{
    //卸载
    i2c_del_driver(&at24c02_drv);
}
module_init(at24c02_init);
module_exit(at24c02_exit);
MODULE_LICENSE("GPL");
