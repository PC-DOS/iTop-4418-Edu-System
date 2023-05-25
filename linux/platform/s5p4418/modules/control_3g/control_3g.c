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

#define DEVICE_NAME				"3g_control"
#define DRIVER_NAME "3g_control"


#define CONTROL_3G_GPIO			(PAD_GPIO_B + 0)

static int __init control_3g_dev_init(void) {
	int ret;

        ret = gpio_request(CONTROL_3G_GPIO, DEVICE_NAME);
        if (ret) {
                printk("request GPIO %d for pwm failed\n", CONTROL_3G_GPIO);
                return ret;
        }

        gpio_direction_output(CONTROL_3G_GPIO, 0);


        printk(DEVICE_NAME "\tcontrol_3g initialized\n");

        return 0;
}

static void __exit control_3g_dev_exit(void) {
	gpio_free(CONTROL_3G_GPIO);

	printk("\tcontrol_3g removed\n");
        return 0;
}

module_init(control_3g_dev_init);
module_exit(control_3g_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("TOPEET Inc.");
MODULE_DESCRIPTION("Control 3G Driver");

