# iTop-4418内核及uBoot源代码修改日志：开机画面

开机画面需要导出为24位色深的Windows BMP格式位图。随后替换`device/nexell/drone2/boot`目录下的`logo.bmp`文件。

`device/nexell/drone2/boot`目录下的`update.bmp`文件描述了系统处于Fastboot状态时显示的图像，亦可一并更改。
