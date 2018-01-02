#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>  //i2c_driver ...
#include <linux/fs.h>  //struct file_opeartions
#include <linux/uaccess.h> //copy_*
#include <linux/miscdevice.h> //struct miscdevice

#define I2C_READ    0x100001 //读AT24C02命令
#define I2C_WRITE   0x100002 //写AT24C02命令

//声明I2C设备驱动和用户应用程序数据交互的结构体
struct eeprom_data {
    unsigned char addr; //片内地址
    unsigned char data; //片内数据
};

//I2C外设的列表
static struct i2c_device_id at24c02_id[] = {
        {"at24c02", 0}, 
        //”at24c02“必须和i2c_board_info的type一致,匹配靠它进行
};

//全局变量
static struct i2c_client *g_client; //记录匹配成功的i2c_client

//从EEPROM读取数据
/*
 *           addr
    app:ioctl----->
        <--------data
                     addr
        at24c02_ioctl----->
                <---------data
                          addr
                    SMBUS----->
                    <---------data
                                    addr
                            总线驱动-----> START 设备地址 写  ACK addr ACK START 设备地址 读 ACK 返回数据data NOACK STOP
                                    <----data
 */
static unsigned char at24c02_i2c_read(unsigned char addr)
{
    /*
       1.使用SMBUS接口将数据(地址和设备地址)丢给I2C总线驱动，启动I2C总线的硬件传输
       1.1打开SMBUS文档:内核源码\Documentation\i2c\smbus-protocol找到对应的SMBUS接口函数
       1.2打开芯片操作时序图
       1.3根据时序图找对应的SMBUS操作函数
       1.4将addr和匹配成功的i2c_client通过函数丢给I2C总线驱动然后启动I2C总线的硬件传输
    */
    unsigned char data; 
    data = i2c_smbus_read_byte_data(g_client, addr);
	return data;
}

//写数据到EEPROM中
/*
 *           addr,data
    app:ioctl--------->  //应用成
                     addr,data
        at24c02_ioctl----------> //I2C设备驱动层
                          addr,data
                    SMBUS----------> //SMBUS接口层
                                    addr,data  //I2C总线驱动层
                            总线驱动---------->
                            			         //硬件层
                            	START 设备地址 写  ACK addr ACK data ACK STOP
 */
static void at24c02_i2c_write(unsigned char addr,
									unsigned char data)
{
    /*
       1.使用SMBUS接口将数据(地址，数据，设备地址（g_client->addr）)丢给I2C总线驱动，启动I2C总线的硬件传输
       1.1打开SMBUS文档:内核源码\Documentation\i2c\smbus-protocol
       找到对应的SMBUS接口函数
       1.2打开芯片操作时序图
       1.3根据时序图找对应的SMBUS操作函数
       1.4将addr,data和匹配成功的i2c_client通过函数丢给I2C总线驱动然后启动I2C总线的硬件传输
    */
    i2c_smbus_write_byte_data(g_client, addr, data);
}

static long at24c02_ioctl(struct file *file,
                            unsigned int cmd,
                            unsigned long arg)
{
	// 1.定义内核缓冲区
    struct eeprom_data eeprom;

    // 2.拷贝用户空间操作的数据信息到内核空间
	
	copy_from_user(&eeprom, 
                (struct eeprom_data *)arg, 
                sizeof(eeprom));

	// 3.解析命令
    switch(cmd) {
            case I2C_READ: //读，用户先得告诉驱动要读哪个地址
			//eeprom.addr = 0x10;
			//eeprom.data = ?
			eeprom.data = at24c02_i2c_read(eeprom.addr);//0x10
			//eeprom.addr = 0x10
			//eeprom.data = 0x55
			//将数据重新告诉给用户
			copy_to_user((struct eeprom_data *)arg,
                                    &eeprom, sizeof(eeprom));
                break;
            case I2C_WRITE://写
                    at24c02_i2c_write(eeprom.addr, //0x10
										eeprom.data);//0x55
                break;
            default:
                return -1;
    }
    return 0;
}

static struct file_operations at24c02_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = at24c02_ioctl
};


//分配初始化miscdevice
static struct miscdevice at24c02_dev = {
    .minor = MISC_DYNAMIC_MINOR, //自动分配次设备号
    .name = "at24c02", //dev/at24c02
    .fops = &at24c02_fops
};

//client指向内核帮咱们通过i2c_board_info实例化的i2c_client
//client里面包含设备地址addr:0x50
static int at24c02_probe(
            struct i2c_client *client, 
            struct i2c_device_id *id)
{
	printk("%s: device addr = %#x\n", 
					__func__, client->addr);
	
    // 1.注册混杂设备驱动,给用户提供访问操作接口
    misc_register(&at24c02_dev); 

	// 2.记录匹配成功的i2c_client
	//局部变量进行全局化
	g_client = client; //指向匹配成功的硬件节点
    return 0; //成功返回0，失败返回负值
}

static int at24c02_remove(struct i2c_client *client) 
{
    //卸载混杂设备
    misc_deregister(&at24c02_dev); 
    return 0; //成功返回0，失败返回负值
}

//定义初始化AT24C02软件节点
static struct i2c_driver at24c02_drv = {
    .driver = {
        .name = "tarena" //不重要，匹配不靠它
    },
    .probe = at24c02_probe, //匹配成功执行
    .remove = at24c02_remove,//卸载调用
    .id_table = at24c02_id
};

static int at24c02_init(void)
{
    //注册i2c_driver
    //添加，遍历，匹配，调用probe
    i2c_add_driver(&at24c02_drv);
    return 0;
}

static void at24c02_exit(void)
{
    //卸载
    //遍历,调用remove
    i2c_del_driver(&at24c02_drv);
}
module_init(at24c02_init);
module_exit(at24c02_exit);
MODULE_LICENSE("GPL");
