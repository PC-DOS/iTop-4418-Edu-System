# iTop-4418内核及uBoot源代码修改日志：自动挂载插入的USB/SD存储设备

## 编译BusyBox

iTop-4418和iTop-4412的Qt/E文件系统已原生支持mdev，无需重新编译BusyBox的以下支持：

```
Linux System Utilities  --->   
       [*] mdev      
             [*]   Support /etc/mdev.conf
             [*]     Support command execution at device addition/removal
```

## 创建脚本

以下操作均把Qt/E文件系统所在的目录视为根目录。

### USB磁盘

创建`/etc/hotplug/`目录，创建`usbdisk`文件，赋予执行权限，写入以下内容：

```
#!/bin/sh

#Judge device action variable ($ACTION)
if [ $ACTION == "add" ]
then
	#Device plugged operations
	echo "Hotplug: USB disk $MDEV plugged." > /dev/console
	if [ -e "/dev/$MDEV" ] ; then
		mkdir -p /mnt/$MDEV
		mount -rw /dev/$MDEV /mnt/$MDEV
		echo "Hotplug: USB disk $MDEV mounted to /mnt/$MDEV ." > /dev/console
	fi
else
	#Device unplugged operations
	echo "Hotplug: USB disk $MDEV unplugged. Unmounting /mnt/$MDEV ..." > /dev/console
	umount /mnt/$MDEV
	rm -r /mnt/$MDEV
fi
```

### SD卡

创建`/etc/hotplug/`目录，创建`sdcard`文件，赋予执行权限，写入以下内容：

```
#!/bin/sh

#Judge device action variable ($ACTION)
if [ $ACTION == "add" ]
then
	#Device plugged operations
	echo "Hotplug: SD card $MDEV plugged." > /dev/console
	if [ -e "/dev/$MDEV" ] ; then
		mkdir -p /mnt/$MDEV
		mount -rw /dev/$MDEV /mnt/$MDEV
		echo "Hotplug: SD card $MDEV mounted to /mnt/$MDEV ." > /dev/console
	fi
else
	#Device unplugged operations
	echo "Hotplug: SD card $MDEV unplugged. Unmounting /mnt/$MDEV ..." > /dev/console
	umount /mnt/$MDEV
	rm -r /mnt/$MDEV
fi
```

## 编辑`mdev.conf`

编辑Qt/E文件系统的`/etc/mdev.conf`，删除：

```
# misc devices
mmcblk0p1	0:0	0600	=sdcard */bin/hotplug
sda1		0:0	0600	=udisk * /bin/hotplug
```

文件末尾添加：

```
# USB disk
sd[a-z][0-9]      0:0 666        * /etc/hotplug/usbdisk

# SD card
mmcblk1p[0-9]     0:0 666        * /etc/hotplug/sdcard
```

## 参考资料

*https://blog.csdn.net/t01051/article/details/107904394