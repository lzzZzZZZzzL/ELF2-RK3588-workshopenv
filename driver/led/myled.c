#include <linux/module.h>       // 包含模块相关函数的头文件
#include <linux/fs.h>           // 包含文件系统相关函数的头文件
#include <linux/uaccess.h>      // 包含用户空间数据访问函数的头文件
#include <linux/cdev.h>         //包含字符设备头文件
#include <linux/device.h>
#include <linux/gpio.h>

#define DEVICE_NAME "mydevice"  // 设备名称
#define LED_IOC_MAGIC 'k'
#define SET_LED_ON _IO(LED_IOC_MAGIC, 0)
#define SET_LED_OFF _IO(LED_IOC_MAGIC, 1)
#define GPIO_LED_PIN_NUM 107
static dev_t dev_num;   //分配的设备号
struct  cdev my_cdev;          //字符设备指针
int major;  //主设备号
int minor;  //次设备号
static struct class *my_led;
static struct device *my_device;

static int device_open(struct inode *inode, struct file *file)
{
// 在这里处理设备打开的操作
//设置GPIO引脚为输出模式，并将其初始化为低电平
gpio_direction_output(GPIO_LED_PIN_NUM, 0);
printk(KERN_INFO "This is device_open.\n");
    return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
// 在这里处理设备关闭的操作
gpio_direction_output(GPIO_LED_PIN_NUM, 0);
printk(KERN_INFO "This is device_release.\n");

    return 0;
}

static long myled_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{

    switch (cmd) {
            case SET_LED_ON:
            // 设置GPIO引脚为高电平
            gpio_set_value(GPIO_LED_PIN_NUM, 1);
            break;

        case SET_LED_OFF:
          //设置GPIO引脚为低电平
          gpio_set_value(GPIO_LED_PIN_NUM, 0);
            break;

        default:
            return -ENOTTY;
    }
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = device_open,
    .release = device_release,
    .unlocked_ioctl = myled_ioctl,
};

static int __init mydevice_init(void)
{
    int ret;

// 在这里执行驱动程序的初始化操作
//释放之前申请的GPIO,避免申请失败
    gpio_free(GPIO_LED_PIN_NUM);
        if (gpio_request(GPIO_LED_PIN_NUM, "led_run")) {
                printk("request %s gpio faile \n", "led_run");
                 return -1;
         }
    // 注册字符设备驱动程序
    ret = alloc_chrdev_region(&dev_num,0,1,DEVICE_NAME);
    if (ret < 0) {
        printk(KERN_ALERT "Failed to allocate device number: %d\n", ret);
        return ret;
}
    major=MAJOR(dev_num);
    minor=MINOR(dev_num);
    printk(KERN_INFO "major number: %d\n",major);
    printk(KERN_INFO "minor number: %d\n",minor);

    my_cdev.owner = THIS_MODULE;
    cdev_init(&my_cdev,&fops);
    cdev_add(&my_cdev,dev_num,1);

    // 创建设备类
    my_led = class_create(THIS_MODULE, "my_led");
    if (IS_ERR(my_led)) {
        pr_err("Failed to create class\n");
        return PTR_ERR(my_led);
    }

    // 创建设备节点并关联到设备类
    my_device = device_create(my_led, NULL, MKDEV(major, minor), NULL, "my_device");
    if (IS_ERR(my_device)) {
        pr_err("Failed to create device\n");
        class_destroy(my_led);
        return PTR_ERR(my_device);
    }

    printk(KERN_INFO "Device registered successfully.\n");
    return 0;
}

static void __exit mydevice_exit(void)
{
    // 在这里执行驱动程序的清理操作
    //释放申请的GPIO资源
    gpio_free(GPIO_LED_PIN_NUM);
    // 销毁设备节点
    device_destroy(my_led, MKDEV(major, minor));
    // 销毁设备类
    class_destroy(my_led);
    // 删除字符设备
    cdev_del(&my_cdev);
    // 注销字符设备驱动程序
    unregister_chrdev(0, DEVICE_NAME);

    printk(KERN_INFO "Device unregistered.\n");
}



module_init(mydevice_init);
module_exit(mydevice_exit);

MODULE_LICENSE("GPL");      // 指定模块的许可证信息
MODULE_AUTHOR("Your Name"); // 指定模块的作者信息
MODULE_DESCRIPTION("A simple character device driver"); // 指定模块的描述信息
