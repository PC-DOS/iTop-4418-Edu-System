# iTop-4418内核及uBoot源代码修改日志：默认启动系统配置

本修改用于支持默认启动Qt/E系统。

## uBoot修改

修改`u-boot/include/configs/s5p4418_drone2.h`文件中`bootsystem`环境变量的默认配置参数定义`CONFIG_BOOT_SYSTEM`为`qt`值。

```
#define CONFIG_BOOT_SYSTEM "qt"
```
