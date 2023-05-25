/*
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

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

#define DEVICE_NAME				"relay_ctl"
#define DRIVER_NAME "relay_ctl"


#define RELAY_GPIO		(PAD_GPIO_B + 18)



static int itopxx18_relay_open(struct inode *inode, struct file *file) {
		return 0;
}

static int itopxx18_relay_close(struct inode *inode, struct file *file) {
	return 0;
}

static long itopxx18_relay_ioctl(struct file *filep, unsigned int cmd,
		unsigned long arg)
{
	printk("%s: cmd = %d\n", __FUNCTION__, cmd);
	switch(cmd) {
		case 0:
			gpio_set_value(RELAY_GPIO, 0);
			break;
		case 1:
			gpio_set_value(RELAY_GPIO, 1);
			break;
		default:
			return -EINVAL;
	}

	return 0;
}

static ssize_t itopxx18_relay_write(struct file *filp, char *buffer, size_t count, loff_t *ppos)
{
	char str[20];

	memset(str, 0, 20);

	if(copy_from_user(str, buffer, count))
	{
		printk("Error\n");

		return -EINVAL;
	}

	printk("%s", str);
#if 1
	if(!strncmp(str, "1", 1))
		gpio_set_value(RELAY_GPIO, 1);
	else
		gpio_set_value(RELAY_GPIO, 0);
#endif
	return count;
}

static struct file_operations itopxx18_relay_ops = {
	.owner			= THIS_MODULE,
	.open			= itopxx18_relay_open,
	.release		= itopxx18_relay_close, 
	.unlocked_ioctl	= itopxx18_relay_ioctl,
	.write			= itopxx18_relay_write,
};

static struct miscdevice itopxx18_misc_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &itopxx18_relay_ops,
};

static int itopxx18_relay_probe(struct platform_device *pdev)
{
	int ret;

	ret = gpio_request(RELAY_GPIO, DEVICE_NAME);
	if (ret) {
		printk("request GPIO %d for relay failed\n", RELAY_GPIO);
		return ret;
	}

	//s3c_gpio_cfgpin(RELAY_GPIO, S3C_GPIO_OUTPUT);
	//gpio_set_value(RELAY_GPIO, 0);
	gpio_direction_output(RELAY_GPIO, 0);

	ret = misc_register(&itopxx18_misc_dev);

	printk(DEVICE_NAME "\tinitialized\n");

	return 0;
}

static int itopxx18_relay_remove (struct platform_device *pdev)
{
	misc_deregister(&itopxx18_misc_dev);
	gpio_free(RELAY_GPIO);	

	return 0;
}

static int itopxx18_relay_suspend (struct platform_device *pdev, pm_message_t state)
{
	printk("relay_ctl suspend:power off!\n");
	return 0;
}

static int itopxx18_relay_resume (struct platform_device *pdev)
{
	printk("relay_ctl resume:power on!\n");
	return 0;
}

static struct platform_driver itopxx18_relay_driver = {
	.probe = itopxx18_relay_probe,
	.remove = itopxx18_relay_remove,
	.suspend = itopxx18_relay_suspend,
	.resume = itopxx18_relay_resume,
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
};

static int __init itopxx18_relay_dev_init(void) {
	return platform_driver_register(&itopxx18_relay_driver);
}

static void __exit itopxx18_relay_dev_exit(void) {
	platform_driver_unregister(&itopxx18_relay_driver);
}

module_init(itopxx18_relay_dev_init);
module_exit(itopxx18_relay_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("TOPEET Inc.");
MODULE_DESCRIPTION("Exynos4 RELAY Driver");

