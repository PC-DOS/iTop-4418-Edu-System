################################################################################
# kernel
################################################################################
PRODUCT_COPY_FILES += \
	kernel/arch/arm/boot/uImage:kernel

################################################################################
# bootloader
################################################################################
#PRODUCT_COPY_FILES += \
	u-boot/u-boot.bin:bootloader

################################################################################
# 2ndboot
################################################################################
#PRODUCT_COPY_FILES += \
	device/nexell/drone/boot/2ndboot.bin:2ndbootloader

################################################################################
# overlay apps
################################################################################
#PRODUCT_COPY_FILES += \
	#hardware/samsung_slsi/slsiap/overlay-apps/GooglePinyinIME.apk:system/app/PinyinIME.apk

################################################################################
# init
################################################################################
PRODUCT_COPY_FILES += \
	device/nexell/drone/init.drone.rc:root/init.drone.rc \
	device/nexell/drone/init.drone.usb.rc:root/init.drone.usb.rc \
	device/nexell/drone/init.recovery.drone.rc:root/init.recovery.drone.rc \
	device/nexell/drone/fstab.drone:root/fstab.drone \
	device/nexell/drone/ueventd.drone.rc:root/ueventd.drone.rc \
	device/nexell/drone/bootanimation.zip:system/media/bootanimation.zip
################################################################################
# key
################################################################################
PRODUCT_COPY_FILES += \
	device/nexell/drone/keypad_drone.kl:system/usr/keylayout/keypad_drone.kl \
	device/nexell/drone/keypad_drone.kcm:system/usr/keychars/keypad_drone.kcm

################################################################################
# touch
################################################################################
PRODUCT_COPY_FILES += \
    device/nexell/drone/gslX680.idc:system/usr/idc/gslX680.idc

################################################################################
# storage
################################################################################
#PRODUCT_COPY_FILES += \
	#device/nexell/drone/vold.fstab:system/etc/vold.fstab

################################################################################
# audio
################################################################################
# mixer paths
PRODUCT_COPY_FILES += \
	device/nexell/drone/audio/tiny_hw.drone.xml:system/etc/tiny_hw.drone.xml
# audio policy configuration
PRODUCT_COPY_FILES += \
	device/nexell/drone/audio/audio_policy.conf:system/etc/audio_policy.conf

################################################################################
# media, camera
################################################################################
PRODUCT_COPY_FILES += \
	device/nexell/drone/media_codecs.xml:system/etc/media_codecs.xml \
	device/nexell/drone/media_profiles.xml:system/etc/media_profiles.xml

################################################################################
# sensor
################################################################################
PRODUCT_PACKAGES += \
	sensors.drone

################################################################################
# camera
################################################################################
PRODUCT_PACKAGES += \
	camera.slsiap

################################################################################
# hwc executable
################################################################################
PRODUCT_PACKAGES += \
	report_hwc_scenario

################################################################################
# modules 
################################################################################
# ogl
PRODUCT_COPY_FILES += \
	hardware/samsung_slsi/slsiap/prebuilt/library/libVR.so:system/lib/libVR.so \
	hardware/samsung_slsi/slsiap/prebuilt/library/libEGL_vr.so:system/lib/egl/libEGL_vr.so \
	hardware/samsung_slsi/slsiap/prebuilt/library/libGLESv1_CM_vr.so:system/lib/egl/libGLESv1_CM_vr.so \
	hardware/samsung_slsi/slsiap/prebuilt/library/libGLESv2_vr.so:system/lib/egl/libGLESv2_vr.so

PRODUCT_COPY_FILES += \
	hardware/samsung_slsi/slsiap/prebuilt/modules/vr.ko:system/lib/modules/vr.ko

# coda
PRODUCT_COPY_FILES += \
	hardware/samsung_slsi/slsiap/prebuilt/modules/nx_vpu.ko:system/lib/modules/nx_vpu.ko

# ffmpeg libraries
EN_FFMPEG_EXTRACTOR := false
EN_FFMPEG_AUDIO_DEC := false
ifeq ($(EN_FFMPEG_EXTRACTOR),true)
PRODUCT_COPY_FILES += \
	hardware/samsung_slsi/slsiap/omx/codec/ffmpeg/libs/libavcodec-2.1.4.so:system/lib/libavcodec-2.1.4.so    \
	hardware/samsung_slsi/slsiap/omx/codec/ffmpeg/libs/libavdevice-2.1.4.so:system/lib/libavdevice-2.1.4.so  \
	hardware/samsung_slsi/slsiap/omx/codec/ffmpeg/libs/libavfilter-2.1.4.so:system/lib/libavfilter-2.1.4.so  \
	hardware/samsung_slsi/slsiap/omx/codec/ffmpeg/libs/libavformat-2.1.4.so:system/lib/libavformat-2.1.4.so  \
	hardware/samsung_slsi/slsiap/omx/codec/ffmpeg/libs/libavresample-2.1.4.so:system/lib/libavresample-2.1.4.so \
	hardware/samsung_slsi/slsiap/omx/codec/ffmpeg/libs/libavutil-2.1.4.so:system/lib/libavutil-2.1.4.so      \
	hardware/samsung_slsi/slsiap/omx/codec/ffmpeg/libs/libswresample-2.1.4.so:system/lib/libswresample-2.1.4.so \
	hardware/samsung_slsi/slsiap/omx/codec/ffmpeg/libs/libswscale-2.1.4.so:system/lib/libswscale-2.1.4.so
endif

# wifi

PRODUCT_COPY_FILES += \
    hardware/samsung_slsi/slsiap/prebuilt/modules/wlan.ko:/system/lib/modules/wlan.ko

################################################################################
# generic
################################################################################
PRODUCT_COPY_FILES += \
  device/nexell/drone/tablet_core_hardware.xml:system/etc/permissions/tablet_core_hardware.xml \
  frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml \
  frameworks/native/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
  frameworks/native/data/etc/android.hardware.wifi.direct.xml:system/etc/permissions/android.hardware.wifi.direct.xml \
  frameworks/native/data/etc/android.hardware.camera.flash-autofocus.xml:system/etc/permissions/android.hardware.camera.flash-autofocus.xml \
  frameworks/native/data/etc/android.hardware.camera.front.xml:system/etc/permissions/android.hardware.camera.front.xml \
  frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml \
  frameworks/native/data/etc/android.hardware.usb.host.xml:system/etc/permissions/android.hardware.usb.host.xml \
  frameworks/native/data/etc/android.hardware.sensor.accelerometer.xml:system/etc/permissions/android.hardware.sensor.accelerometer.xml \
  frameworks/native/data/etc/android.hardware.audio.low_latency.xml:system/etc/permissions/android.hardware.audio.low_latency.xml \
  linux/platform/s5p4418/library/lib/ratecontrol/libnxvidrc_android.so:system/lib/libnxvidrc_android.so

PRODUCT_AAPT_CONFIG := xlarge hdpi xhdpi large
PRODUCT_AAPT_PREF_CONFIG := xhdpi

# 4330 delete nosdcard
# PRODUCT_CHARACTERISTICS := tablet,nosdcard
PRODUCT_CHARACTERISTICS := tablet,usbstorage

DEVICE_PACKAGE_OVERLAYS := \
	device/nexell/drone/overlay

PRODUCT_TAGS += dalvik.gc.type-precise

PRODUCT_PACKAGES += \
	librs_jni \
	com.android.future.usb.accessory

PRODUCT_PACKAGES += \
	audio.a2dp.default \
	audio.usb.default \
	audio.r_submix.default

# Filesystem management tools
PRODUCT_PACKAGES += \
    e2fsck

# Linaro
PRODUCT_PACKAGES += \
	GLMark2 \
	libglmark2-android

# Product Property
# common
PRODUCT_PROPERTY_OVERRIDES := \
	wifi.interface=wlan0 \
	ro.sf.lcd_density=160

# 4330 openl ui property
#PRODUCT_PROPERTY_OVERRIDES += \
	ro.opengles.version=131072 \
	ro.hwui.texture_cache_size=72 \
	ro.hwui.layer_cache_size=48 \
	ro.hwui.path_cache_size=16 \
	ro.hwui.shape_cache_size=4 \
	ro.hwui.gradient_cache_size=1 \
	ro.hwui.drop_shadow_cache_size=6 \
	ro.hwui.texture_cache_flush_rate=0.4 \
	ro.hwui.text_small_cache_width=1024 \
	ro.hwui.text_small_cache_height=1024 \
	ro.hwui.text_large_cache_width=2048 \
	ro.hwui.text_large_cache_height=1024 \
	ro.hwui.disable_scissor_opt=true

# setup dalvik vm configs.
#$(call inherit-product, frameworks/native/build/tablet-10in-xhdpi-2048-dalvik-heap.mk)
$(call inherit-product, frameworks/native/build/tablet-7in-hdpi-1024-dalvik-heap.mk)

# The OpenGL ES API level that is natively supported by this device.
# This is a 16.16 fixed point number
PRODUCT_PROPERTY_OVERRIDES += \
	ro.opengles.version=131072

# Enable AAC 5.1 output
#PRODUCT_PROPERTY_OVERRIDES += \
	media.aac_51_output_enabled=true

# set default USB configuration
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
	persist.sys.usb.config=mtp

# ota updater test
PRODUCT_PACKAGES += \
	OTAUpdateCenter

# miracast sink
 PRODUCT_PACKAGES += \
	Mira4U

# wifi
ifeq ($(BOARD_WIFI_VENDOR),realtek)
$(call inherit-product-if-exists, hardware/realtek/wlan/config/p2p_supplicant.mk)
endif

ifeq ($(BOARD_WIFI_VENDOR),broadcom)
WIFI_BAND := 802_11_BG
$(call inherit-product-if-exists, hardware/broadcom/wlan/bcmdhd/firmware/bcm4329/device-bcm.mk)
endif

# 3G/LTE

# call slsiap
$(call inherit-product-if-exists, hardware/samsung_slsi/slsiap/slsiap.mk)

# google gms
#$(call inherit-product-if-exists, vendor/google/gapps/device-partial.mk)
