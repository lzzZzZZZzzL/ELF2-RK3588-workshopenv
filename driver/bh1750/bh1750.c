#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/i2c.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <asm/io.h>
#include <linux/device.h>
#include <linux/platform_device.h>

#define DEV_NAME "bh1750"
#define DEV_CNT (1)

#define POWERON				0x01
#define POWERDOWN			0x0
#define RESET				0x7
#define CON_H_RESOLUTION_MODE		0x10
#define CON_H_RESOLUTION_MODE2		0x11
#define CON_L_RESOLUTION_MODE		0x13
#define ONE_H_RESOLUTION_MODE		0x20
#define ONE_H_RESOLUTION_MODE2		0x21
#define ONE_L_RESOLUTION_MODE		0x23

typedef struct {
	dev_t devid;
	struct cdev cdev;
	struct class *class;
	struct device *device;
	struct device_node *nd;
	int major;
	void *private_data;
}bh1750_dev_t;

bh1750_dev_t bh1750dev;


static int bh1750_read_data(bh1750_dev_t *dev , void *val , int len)
{
	struct i2c_msg msg[2];
	struct i2c_client *client = (struct i2c_client*)dev->private_data;

	msg[0].addr = client->addr;	
	msg[0].flags = I2C_M_RD;	
	msg[0].buf = val;	
	msg[0].len = len;		

	return i2c_transfer(client->adapter, msg, 1);
}

static int bh1750_write_cmd(bh1750_dev_t *dev , unsigned char cmd)
{
	struct i2c_msg msg;
        struct i2c_client *client = (struct i2c_client*)dev->private_data;
	msg.addr = client->addr;
	msg.flags = 0;
	msg.buf = &cmd;
	msg.len = 1;
	return i2c_transfer(client->adapter, &msg, 1);
}

static int bh1750_open(struct inode *inode, struct file *filp)
{
	filp->private_data = &bh1750dev;
	return 0;
}

static ssize_t bh1750_read(struct file *filp, char __user *buf, size_t cnt, loff_t *off)
{
	int ret;
	unsigned char _data[2];
	unsigned short sensor_data = 0;
	bh1750_dev_t *dev = filp->private_data;

	bh1750_write_cmd(dev,POWERON); 
	bh1750_write_cmd(dev,CON_H_RESOLUTION_MODE);
	mdelay(180);
	bh1750_read_data(dev,_data,2);
	
	sensor_data = *_data;
	sensor_data = (sensor_data << 8) + *(_data + 1);
	ret = copy_to_user(buf,&sensor_data,2);

	return ret;

}

static int bh1750_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static void bh1750dev_init(bh1750_dev_t *dev)
{
	bh1750_write_cmd(dev,POWERON);
}

static struct file_operations bh1750_chr_dev_fops =
{
	.owner = THIS_MODULE,
	.open = bh1750_open,
	.read = bh1750_read,
	.release = bh1750_release,
};

static int bh1750_probe(struct i2c_client *client,const struct i2c_device_id *id)
{
	int ret = -1;
	ret = alloc_chrdev_region(&bh1750dev.devid, 0, DEV_CNT, DEV_NAME);
	if (ret < 0)
	{
		printk("fail to alloc bh1750_dev\n");
		goto alloc_err;
	}

	cdev_init(&bh1750dev.cdev, &bh1750_chr_dev_fops);
	ret = cdev_add(&bh1750dev.cdev, bh1750dev.devid, DEV_CNT);
	if (ret < 0)
	{
		printk("fail to add cdev\n");
		goto add_err;
	}

	bh1750dev.class = class_create(THIS_MODULE, DEV_NAME);
	bh1750dev.device = device_create(bh1750dev.class, NULL, bh1750dev.devid, NULL, DEV_NAME);
	bh1750dev.private_data = client;
	bh1750dev_init(&bh1750dev);
	return 0;

add_err:
	unregister_chrdev_region(bh1750dev.devid, DEV_CNT);
	printk("\n add_err error! \n");
alloc_err:
	return ret;
}

static int bh1750_remove(struct i2c_client *client)
{
	device_destroy(bh1750dev.class, bh1750dev.devid);
	class_destroy(bh1750dev.class);
	cdev_del(&bh1750dev.cdev);
	unregister_chrdev_region(bh1750dev.devid, DEV_CNT);
	return 0;
}

static struct of_device_id bh1750_of_match[] = {
	{.compatible = "elfboard,bh1750"},
	{},
};

static struct i2c_device_id bh1750_id[] = {
	{"elfboard,bh1750",0},
	{},
};

static struct i2c_driver bh1750_driver = {
	.probe = bh1750_probe,
	.remove = bh1750_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = "bh1750",
		.of_match_table = bh1750_of_match,
	},
	.id_table = bh1750_id,
};

static int __init bh1750_init(void)
{
	int ret = 0;
	ret = i2c_add_driver(&bh1750_driver);
	return ret;
}

static void __exit bh1750_exit(void)
{
	i2c_del_driver(&bh1750_driver);
}

module_init(bh1750_init);
module_exit(bh1750_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("YourName");
MODULE_DESCRIPTION("bh1750 sensor driver");
