# iTop-4418内核及uBoot源代码修改日志

迅为iTop-4418开发板的源码文件将编译器、Android 4.4、Linux内核、uBoot源码、文件系统构成了一个大包。

`u-boot`符号链接指向uBoot源码。

`kernel`符号链接指向Linux内核源码。

`linux`目录下存放了内核、uBoot源码。

`qt_system`目录下存放了Qt/E文件系统。

## 移除Android源代码

迅为iTop-4418开发板的源码文件将编译器、Android 4.4、Linux内核、uBoot源码构成了一个大包，如果您不需要Android源代码，可以进入解包源码包后得到的`android`目录中，删除以下目录：

```
abi
art
bionic
cts
dalvik
device/asus
device/common
device/generic
device/google
device/lge
device/nexell/.git
device/nexell/tools/android-sdk_r22.3-linux.tgz
device/sample
device/samsung
developers
development
docs
external
frameworks
hardware/akm
hardware/broadcom
hardware/espressif
hardware/invensense
hardware/libhardware
hardware/libhardware_legacy
hardware/mediatek
hardware/qcom
hardware/realtek
hardware/ril
hardware/ti
libcore
libnativehelper
linux/platform/s5p4418/tools/crosstools/arm-cortex_a9-eabi-4.7-eglibc-2.18.tar.gz
linux/prototype/s5p4418/.git
linux/prototype/s5p6818/.git
ndk
packages
pdk
prebuilts/clang
prebuilts/devtools
prebuilts/eclipse
prebuilts/ndk
prebuilts/python
prebuilts/qemu-kernel
prebuilts/runtime
prebuilts/sdk
qt_system/linux-x86/bin/clang
qt_system/linux-x86/bin/clang++
sdk
system
vendor/google
```

随后，将`android`目录中的build_android.sh的下列代码移除或通过在行首加“`#`”的方式进行注释，跳过对Android的编译。

```
if [ "$kernel_type" = "android" ]
then
    build_android
fi
```

## 编译

修改`build_android.sh`文件，注释掉下列代码：

```
if [ "$kernel_type" = "android" ]
then
	build_android
fi
```

随后在终端执行：

```
./build_android.sh qt
```

## uBoot配置

### 三星 1G DDR3 内存

在终端中执行：

```
cd u-boot
cp 2ndboot_sdmmc_4418_samsung.bin 2ndboot_sdmmc_4418.bin
cp nsih-1G16b-4418.txt nsih.txt
```

### 三星 2G DDR3 内存

在终端中执行：

```
cd u-boot
cp 2ndboot_sdmmc_4418_samsung.bin 2ndboot_sdmmc_4418.bin
cp nsih-2G16b-4418.txt nsih.txt
```

### 镁光 2G DDR3 内存

在终端中执行：

```
cd u-boot
cp 2ndboot_sdmmc_4418_micron.bin 2ndboot_sdmmc_4418.bin
cp nsih-2G16b-4418_micron.txt nsih.txt
```

## Linux内核配置

在终端执行：

```
cd kernel
export ARCH=arm
make menuconfig
```

此外，iTop-4418随附了一些默认的配置文件，位于`kernel`目录下：

`config_for_iTOP4418_android_AR8031`：支持 Android，以太网卡型号为 AR8031 的缺省文件。

`config_for_iTOP4418_android_RTL8211`：支持 Android，以太网卡型号为 RTL8211 的缺省文件。

`config_for_iTOP4418_linux_AR8031`：支持 Qt，以太网卡型号为 AR8031 的缺省文件。

`config_for_iTOP4418_linux_RTL8211`：支持 Qt，以太网卡型号为 RTL8211 的缺省文件。

使用`cp -r config_for_iTOP4418_linux_RTL8211 .config`这样的指令可以套用默认配置文件。

对于使用2 GB内存的核心板，需要编辑`kernel/arch/arm/plat-s5p4418/topeet/include/cfg_mem.h`文件，将`#define CFG_MEM_PHY_SYSTEM_SIZE`处的`#if 1`改为`#if 0`。

## 配置记录

[显示器配置](ModificationLog/iTop-4418-Screen-Config-Log.md)

[默认启动系统设置](ModificationLog/iTop-4418-Default-System-Config-Log.md)

[开机画面修改](ModificationLog/iTop-4418-Boot-Splash-Screen-Config-Log.md)

[Qt/E 4.8.7编译、安装与配置](ModificationLog/iTop-4418-QtE-4-8-7-Config-Log.md)

[移除厂商驱动程序中的输出文本](ModificationLog/iTop-4418-Remove-OEM-Driver-Print.md)

[开机振铃](ModificationLog/iTop-4418-Power-On-Beep.md)

[配置Qt/E或最小Linux以自动挂载插入的USB/SD存储设备](ModificationLog/iTop-4418-Storage-Device-Auto-Mounting-Config-Log.md)

[配置从eMMC（SD2）启动](ModificationLog/iTop-4418-eMMC-Boot-Config-Log.md)

[Qt/E环境变量全局化](ModificationLog/iTop-4418-QtE-Environment-Variables-Config-Log.md)
