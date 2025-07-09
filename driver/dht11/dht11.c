/*
模块名称：dht11.h
摘    要：dht11模块测温湿度
*/
/*包含头文件*/
#include <linux/module.h>       // 包含模块相关函数的头文件
#include <linux/fs.h>           // 包含文件系统相关函数的头文件
#include <linux/uaccess.h>      // 包含用户空间数据访问函数的头文件
#include <linux/cdev.h>         //包含字符设备头文件
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include "dht11.h"
//宏定义

//内部变量
static dev_t dev_num;       //分配的设备号
struct cdev DHT11_cdev;     //字符设备指针
int major;  //主设备号
int minor;  //次设备号
static struct class *DHT11_class;
static struct device *DHT11_device;
//函数声明
static u8 DHT11_data[5];
static void DHT11_Mode(unsigned char mode);
static void DHT11_Rst(void);
static u8 DHT11_Check(void);
static u8 DHT11ReadBit(void);
static u8 DHT11ReadByte(void);
//函数实现
static void DHT11_Rst(void)	//复位DHT11
{
    DHT11_Mode(1);
    gpio_set_value(DHT11_GPIO_PIN_NUM, 0);
    mdelay(20);
    gpio_set_value(DHT11_GPIO_PIN_NUM, 1);
    udelay(13);
}
static u8 DHT11_Check(void)	//等待DHT11的回应
{
    unsigned char retry = 0;
    DHT11_Mode(0);
    while (gpio_get_value(DHT11_GPIO_PIN_NUM) && retry < 90)	//DHT11会拉低40~80us
    {
        retry++;
        udelay(1);
    }
    if (retry >= 90)
    {
    	printk("DHT11_Check retry >= 90! retry=%d\n\n",retry);
        return 1;
    }
    else
        retry = 0;
    while (!gpio_get_value(DHT11_GPIO_PIN_NUM) && retry < 92)	//DHT11拉低后会再次拉高40~80us
    {
        retry++;
        udelay(1);
    }
    if (retry >= 92)
    {
    	printk("DHT11_Check retry >= 92! retry=%d\n\n",retry);
    	return 1;
    }
    return 0;
}
static u8 DHT11ReadBit(void)		//从DHT11读取一个位
{
    unsigned char retry = 0;
    while (gpio_get_value(DHT11_GPIO_PIN_NUM) && retry < 100)	//等待变为低电平
    {
        retry++;
        udelay(1);
    }
    retry = 0;
    while (!gpio_get_value(DHT11_GPIO_PIN_NUM) && retry < 100)	//等待变高电平
    {
        retry++;
        udelay(1);
    }
    udelay(40);
    if (gpio_get_value(DHT11_GPIO_PIN_NUM))
    {
    	return 1;
    }
    else
        return 0;
}
static u8 DHT11ReadByte(void)
{
    unsigned char i, dat;
    dat = 0;
    for (i = 0; i < 8; i++)
    {
        dat <<= 1;
        dat |= DHT11ReadBit();
    }
    return dat;
}

void DHT11_Mode(u8 mode)	//mode=0则为输入模式
{
    if (mode)
        gpio_direction_output(DHT11_GPIO_PIN_NUM, 1);
    else
        gpio_direction_input(DHT11_GPIO_PIN_NUM);
}

static long DHT11_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int i = 0;
    switch (cmd)
    {
        case DHT11_READ_DATA:
            DHT11_Rst();
            if (DHT11_Check() == 0)
            {
                for (i = 0; i < 5; i++)
                {
                    DHT11_data[i] = DHT11ReadByte();
                }
                if ((DHT11_data[0] + DHT11_data[1] + DHT11_data[2] + DHT11_data[3]) == DHT11_data[4])
                {
                    DHT11_data[4] = 1;
                }
                else
                {
                    DHT11_data[4] = -1;
                    printk("Data read error!\n\n");
                }
            }
            else
            {
                DHT11_data[0] = 0;
                printk("Can't use dht11!\n\n");
            }
            break;
        default:
            return -ENOTTY;
    }
    if (copy_to_user((unsigned char __user *)arg, DHT11_data, sizeof(DHT11_data)))
    {
        return -EFAULT;
    }
    return 0;
}
static int DHT11_open(struct inode *inode, struct file *file)
{
    return 0;
}
static int DHT11_release(struct inode *inode, struct file *file)
{
    return 0;
}
static struct file_operations fops =
{
    .owner = THIS_MODULE,
    .open = DHT11_open,
    .release = DHT11_release,
    .unlocked_ioctl = DHT11_ioctl,
};
static int __init DHT11_init(void)
{
    int ret;
    gpio_free(DHT11_GPIO_PIN_NUM);
    if (gpio_request(DHT11_GPIO_PIN_NUM, "DHT11"))
    {
        printk("request %s gpio faile \n", "DHT11");
        return -1;
    }
    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0)
    {
        printk(KERN_ALERT "Failed to allocate device number: %d\n", ret);
        return ret;
    }
    major = MAJOR(dev_num);
    minor = MINOR(dev_num);
    printk(KERN_INFO "major number: %d\n", major);
    printk(KERN_INFO "minor number: %d\n", minor);
    DHT11_cdev.owner = THIS_MODULE;
    cdev_init(&DHT11_cdev, &fops);
    cdev_add(&DHT11_cdev, dev_num, 1);
    DHT11_class = class_create(THIS_MODULE, "DHT11");
    if (IS_ERR(DHT11_class))
    {
        pr_err("Failed to create class\n");
        return PTR_ERR(DHT11_class);
    }
    DHT11_device = device_create(DHT11_class, NULL, MKDEV(major, minor), NULL, DEVICE_NAME);
    if (IS_ERR(DHT11_device))
    {
        pr_err("Failed to create device\n");
        class_destroy(DHT11_class);
        return PTR_ERR(DHT11_device);
    }
    printk(KERN_INFO "Device registered successfully.\n");
    return 0;
}
static void __exit DHT11_exit(void)
{
    gpio_free(DHT11_GPIO_PIN_NUM);
    device_destroy(DHT11_class, MKDEV(major, minor));
    class_destroy(DHT11_class);
    cdev_del(&DHT11_cdev);
    unregister_chrdev_region(dev_num, 1);
    printk(KERN_INFO "Device unregistered.\n");
}
module_init(DHT11_init);
module_exit(DHT11_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple DHT11 driver");
