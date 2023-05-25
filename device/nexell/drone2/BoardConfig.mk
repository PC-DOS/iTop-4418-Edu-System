TARGET_BOARD_INFO_FILE := device/nexell/drone2/board-info.txt


TARGET_BOARD_PLATFORM 		 := slsiap
TARGET_BOOTLOADER_BOARD_NAME := drone2

TARGET_CPU_ABI  := armeabi-v7a
TARGET_CPU_ABI2 := armeabi
TARGET_CPU_SMP  := true
TARGET_ARCH 	:= arm
TARGET_ARCH_VARIANT := armv7-a-neon
TARGET_CPU_VARIANT := cortex-a9
ARCH_ARM_HAVE_TLS_REGISTER  := true

# chip variation
TARGET_CPU_VARIANT2 := s5p4418

TARGET_NO_BOOTLOADER := false
TARGET_NO_KERNEL 	 := false
TARGET_BOOTLOADER_IS_2ND := false
TARGET_NO_RADIOIMAGE := false

# recovery
TARGET_RECOVERY_FSTAB := device/nexell/drone2/recovery.fstab
TARGET_RELEASETOOLS_EXTENSIONS := device/nexell/drone2
TARGET_RECOVERY_UPDATER_LIBS += librecovery_updater_drone2
TARGET_RECOVERY_UI_LIB := librecovery_ui_drone2
# TARGET_RECOVERY_PIXEL_FORMAT not specified ==> rgb565

# 2ndbootloader, bootloader
#$(call add-radio-file,2ndbootloader)
#$(call add-radio-file,bootloader)
#INSTALLED_RADIOIMAGE_TARGET += 2ndbootloader
#INSTALLED_RADIOIMAGE_TARGET += bootloader
#$(warning INSTALLED_RADIOIMAGE_TARGET: $(INSTALLED_RADIOIMAGE_TARGET))

# certificate
#PRODUCT_DEFAULT_DEV_CERTIFICATE := vendor/nexell/security/drone2/release

# opengl
BOARD_EGL_CFG := device/nexell/drone2/egl.cfg

USE_OPENGL_RENDERER := true
NUM_FRAMEBUFFER_SURFACE_BUFFERS := 3

# audio
BOARD_USES_GENERIC_AUDIO := false
BOARD_USES_ALSA_AUDIO 	 := false

BOARD_HAVE_BLUETOOTH := false

# GPS
BOARD_HAS_GPS := true
BOARD_GPS_LIBRARIES := libgps
BOARD_HAVE_HARDWARE_GPS := true
USE_QEMU_GPS_HARDWARE := false

# camera
BOARD_HAS_CAMERA := true

# sensor
BOARD_HAS_SENSOR := true
#modify by cym 20151215 for MMA8451
#BOARD_SENSOR_TYPE := general
BOARD_SENSOR_TYPE := invense
#end modify

# external ffmpeg plugin
EN_FFMPEG_EXTRACTOR := true
EN_FFMPEG_AUDIO_DEC := true


# wifi
# broadcom bcm4329
# realtek 8188eu
#BOARD_WIFI_VENDOR := realtek
#ifeq ($(BOARD_WIFI_VENDOR),broadcom)
#BOARD_WPA_SUPPLICANT_DRIVER	:= NL80211
#WPA_SUPPLICANT_VERSION		:= VER_0_8_X
#BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_bcmdhd
#BOARD_HOSTAPD_DRIVER		:= NL80211
#BOARD_HOSTAPD_PRIVATE_LIB	:= lib_driver_cmd_bcmdhd
#BOARD_WLAN_DEVICE		:= bcmdhd
#WIFI_DRIVER_FW_PATH_PARAM	:= "/sys/module/bcmdhd/parameters/firmware_path"
#WIFI_DRIVER_FW_PATH_STA		:= "/vendor/firmware/fw_bcmdhd.bin"
#WIFI_DRIVER_FW_PATH_P2P		:= "/vendor/firmware/fw_bcmdhd_p2p.bin"
#WIFI_DRIVER_FW_PATH_AP		:= "/vendor/firmware/fw_bcmdhd_apsta.bin"
#endif

#ifeq ($(BOARD_WIFI_VENDOR),realtek)
#WPA_SUPPLICANT_VERSION		:= VER_0_8_X
#BOARD_WPA_SUPPLICANT_DRIVER	:= NL80211
#CONFIG_DRIVER_WEXT		:=y
#BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_rtl
#BOARD_HOSTAPD_DRIVER		:= NL80211
#BOARD_HOSTAPD_PRIVATE_LIB	:= lib_driver_cmd_rtl

#BOARD_WLAN_DEVICE	:= rtl8188eu

#WIFI_DRIVER_MODULE_NAME		:= "wlan"
#WIFI_DRIVER_MODULE_PATH		:= "/system/lib/modules/wlan.ko"
#WIFI_DRIVER_MODULE_ARG		:= "ifname=wlan0 if2name=p2p0"

#WIFI_FIRMWARE_LOADER		:= ""
#WIFI_DRIVER_FW_PATH_STA		:= ""
#WIFI_DRIVER_FW_PATH_AP		:= ""
#WIFI_DRIVER_FW_PATH_P2P		:= ""
#WIFI_DRIVER_FW_PATH_PARAM	:= ""
#endif


# wifi mt6620 
BOARD_WIFI_VENDOR	:= MediaTek
BOARD_CONNECTIVITY_VENDOR := MediaTek

ifeq ($(strip $(BOARD_CONNECTIVITY_VENDOR)), MediaTek)
BOARD_MEDIATEK_USES_GPS  := false
BOARD_MEDIATEK_USES_FM  := false
BOARD_CONNECTIVITY_MODULE := combo_mt66xx

ifeq ($(strip $(BOARD_CONNECTIVITY_MODULE)), combo_mt66xx)
combo_config :=  hardware/mediatek/config/$(strip $(BOARD_CONNECTIVITY_MODULE))/board_config.mk
include  $(combo_config)
endif

endif




# RIL
# BOARD_HAS_RIL := true

# HWC
SLSIAP_HWC_VERSION := 2

# sepolicy
BOARD_SEPOLICY_DIRS := \
	device/nexell/drone2/sepolicy

BOARD_SEPOLICY_UNION := \
	file_contexts \
	genfs_contexts \
	adbd.te \
	app.te \
	device.te \
	domain.te \
	file.te \
	mediaserver.te \
	surfaceflinger.te \
	system.te

# for google gms
#-include vendor/google/gapps/BoardConfigPartial.mk

# packaging for emmc, sd
TARGET_USERIMAGES_USE_EXT4       := true
BOARD_CACHEIMAGE_FILE_SYSTEM_TYPE := ext4
BOARD_BOOTIMAGE_PARTITION_SIZE := 67108864
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 685768704
BOARD_CACHEIMAGE_PARTITION_SIZE  := 553648128
#BOARD_USERDATAIMAGE_PARTITION_SIZE := 6000000000
BOARD_USERDATAIMAGE_PARTITION_SIZE := 14292090368
BOARD_FLASH_BLOCK_SIZE           := 4096
