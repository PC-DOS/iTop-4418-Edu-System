#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <asm/mach-types.h>
#include <linux/gpio.h>
#include <asm/gpio.h>
#include <asm/delay.h>
#include <linux/clk.h>
#include <mach/gpio.h>
#include <mach/soc.h>
#include <mach/platform.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

//#define GPS_DEBUG
#ifdef GPS_DEBUG
#define DPRINTK(x...) printk("MAX485_CTL DEBUG:" x)
#else
#define DPRINTK(x...)
#endif

#define DEVICE_NAME				"max485_ctl"
#define DRIVER_NAME "max485_ctl"

#define MAX485_GPIO			(PAD_GPIO_C + 6)
int max485_ctl_open(struct inode *inode,struct file *filp)
{
	DPRINTK("Device Opened Success!\n");
	return nonseekable_open(inode,filp);
}

int max485_ctl_release(struct inode *inode,struct file *filp)
{
	DPRINTK("Device Closed Success!\n");
	return 0;
}

int max485_ctl_pm(bool enable)
{
	int ret = 0;
	printk("firecxx debug: GPS PM return %d\r\n" , ret);
	return ret;
};

long max485_ctl_ioctl(struct file *filp,unsigned int cmd,unsigned long arg)
{
	printk("firecxx debug: max485_ctl_ioctl cmd is %d\n" , cmd);

	switch(cmd)
	{		
		case 1:
			if(gpio_request(MAX485_GPIO ,"DEVICE_NAME"))
			{
				DPRINTK("max485_ctl GPIO err!\r\n");
			}
			else
			{
				gpio_direction_output(MAX485_GPIO, 1);
				DPRINTK("max485_ctl Set High!\n");
				gpio_free(MAX485_GPIO);

				mdelay(100);
			}
				
			break;
		case 0:
			if(gpio_request(MAX485_GPIO ,"DEVICE_NAME"))
			{
				DPRINTK("max485_ctl GPIO err!\r\n");
			}
			else
			{			
				gpio_direction_output(MAX485_GPIO,0);
				DPRINTK("max485_ctl Set Low!\n");
				gpio_free(MAX485_GPIO);

				mdelay(100); 
			}
			
			break;
			
		default:
			DPRINTK("max485_ctl COMMAND ERROR!\n");
			return -ENOTTY;
	}
	return 0;
}

static struct file_operations max485_ctl_ops = {
	.owner 	= THIS_MODULE,
	.open 	= max485_ctl_open,
	.release= max485_ctl_release,
	.unlocked_ioctl 	= max485_ctl_ioctl,
};

static struct miscdevice max485_ctl_dev = {
	.minor	= MISC_DYNAMIC_MINOR,
	.fops	= &max485_ctl_ops,
	.name	= "max485_ctl_pin",
};


static int max485_ctl_probe(struct platform_device *pdev)
{
	int err = 0;
	
	int ret;
	char *banner = "max485_ctl Initialize\n";

	printk(banner);
	gpio_free(MAX485_GPIO);
	err = gpio_request(MAX485_GPIO, "DEVICE_NAME");
	if (err) {
		printk(KERN_ERR "failed to request DEVICE_NAME for "
			"max485_ctl control\n");
		return err;
	}
	gpio_direction_output(MAX485_GPIO, 1);

	//s3c_gpio_cfgpin(MAX485_GPIO, S3C_GPIO_OUTPUT);
		gpio_direction_output(MAX485_GPIO, 0);
		gpio_free(MAX485_GPIO);
     
	  
	ret = misc_register(&max485_ctl_dev);
	if(ret<0)
	{
		printk("max485_ctl:register device failed!\n");
		goto exit;
	}

	return 0;

exit:
	misc_deregister(&max485_ctl_dev);
	return ret;
}

static int max485_ctl_remove (struct platform_device *pdev)
{
	misc_deregister(&max485_ctl_dev);	

	return 0;
}

static int max485_ctl_suspend (struct platform_device *pdev, pm_message_t state)
{
	DPRINTK("max485_ctl suspend:power off!\n");
	return 0;
}

static int max485_ctl_resume (struct platform_device *pdev)
{
	DPRINTK("max485_ctl resume:power on!\n");
	return 0;
}

static struct platform_driver max485_ctl_driver = {
	.probe = max485_ctl_probe,
	.remove = max485_ctl_remove,
	.suspend = max485_ctl_suspend,
	.resume = max485_ctl_resume,
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
};

static void __exit max485_ctl_exit(void)
{
	platform_driver_unregister(&max485_ctl_driver);
}

static int __init max485_ctl_init(void)
{	
	return platform_driver_register(&max485_ctl_driver);
}

module_init(max485_ctl_init);
module_exit(max485_ctl_exit);

MODULE_LICENSE("Dual BSD/GPL");
