# iTop-4418内核及uBoot源代码修改日志：电源状态锁存与上电振铃

## 上电振铃

设备开机时，鸣响蜂鸣器作为设备通电自检和正常引导的标志。蜂鸣器通过GPIO引脚组C的14号引脚（GPIOC14）连接到核心板。当该引脚被设为高电平（1）时，蜂鸣器持续鸣响；当该引脚被设为低电平（0）时，蜂鸣器停止鸣响。

鸣响蜂鸣器在uBoot对设备板进行初始化时执行，相关的代码文件位于uBoot代码仓库的板级支持包代码库中的`u-boot/board/s5p4418/drone2/board.c`文件中，通过修改该文件中的`board_init()`板初始化函数，并在该函数中添加以下流程，即可实现开机时蜂鸣器的鸣响：

```c++
//Modification: Beep when powering up, added by Picsell Dois (T.C.D.)
gpio_direction_output(PAD_GPIO_C + 14, 1);
mdelay(500);
gpio_direction_output(PAD_GPIO_C + 14, 0);
//End-of-Modification: Beep when powering up, added by Picsell Dois (T.C.D.)
```
