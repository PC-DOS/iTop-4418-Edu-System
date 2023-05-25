# iTop-4418内核及uBoot源代码修改日志：移除厂商驱动程序中的输出

## 蜂鸣器驱动程序

文件位于`kernel/drivers/char/itop4418_buzzer.c`，开启后，移除`itop4418_buzzer_ioctl()`函数和`itop4418_buzzer_write()`函数中的`printk()`调用即可。

## LED驱动程序

文件位于`kernel/drivers/char/itop4418_led.c`，开启后，移除`itop4418_led_ioctl()`函数和`itop4418_led_write()`函数中的`printk()`调用即可。

## 中断调试信息

文件位于`kernel/arch/arm/mach-s5p4418/irq.c`，开启后，移除`gpio_handler()`函数中的`printk()`调用即可。
