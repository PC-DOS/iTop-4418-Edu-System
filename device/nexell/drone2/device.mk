################################################################################
# kernel
################################################################################
PRODUCT_COPY_FILES += \
	kernel/arch/arm/boot/uImage:kernel

################################################################################
# init
################################################################################
PRODUCT_COPY_FILES += \
	device/nexell/drone2/init.drone2.rc:root/init.drone2.rc \
	device/nexell/drone2/init.drone2.usb.rc:root/init.drone2.usb.rc \
	device/nexell/drone2/init.recovery.drone2.rc:root/init.recovery.drone2.rc \
	device/nexell/drone2/fstab.drone2:root/fstab.drone2 \
	device/nexell/drone2/ueventd.drone2.rc:root/ueventd.drone2.rc \
#remove by cym 20150812 	device/nexell/drone2/bootanimation.zip:system/media/bootanimation.zip

################################################################################
# key
################################################################################
PRODUCT_COPY_FILES += \
	device/nexell/drone2/keypad_drone2.kl:system/usr/keylayout/keypad_drone2.kl \
	device/nexell/drone2/keypad_drone2.kcm:system/usr/keychars/keypad_drone2.kcm

################################################################################
# touch
################################################################################
PRODUCT_COPY_FILES += \
    device/nexell/drone2/gslX680.idc:system/usr/idc/gslX680.idc

#add by cym 20150812
PRODUCT_COPY_FILES += \
    device/nexell/drone2/ft5x0x_ts.idc:system/usr/idc/ft5x0x_ts.idc \
    device/nexell/drone2/Goodix-TS.idc:system/usr/idc/Goodix-TS.idc
#end add

#add by cym 20150922 for ZTE Welink MW3820
#PRODUCT_PACKAGES += \
#        ip-up-ppp0

#PRODUCT_COPY_FILES += \
    device/nexell/drone2/nw3820/apns-full-conf.xml:system/etc/apns-conf.xml \
    device/nexell/drone2/nw3820/spn-conf.xml:system/etc/spn-conf.xml \
    device/nexell/drone2/nw3820/init.gprs-pppd:system/etc/init.gprs-pppd \
    device/nexell/drone2/nw3820/libreference-ril.so:system/lib/libreference-ril.so
#end add

PRODUCT_COPY_FILES += \
    device/nexell/drone2/ec20/apns-full-conf.xml:system/etc/apns-conf.xml \
    device/nexell/drone2/ec20/spn-conf.xml:system/etc/spn-conf.xml \

################################################################################
# audio
################################################################################
# mixer paths
PRODUCT_COPY_FILES += \
	device/nexell/drone2/audio/tiny_hw.drone2.xml:system/etc/tiny_hw.drone2.xml
# audio policy configuration
PRODUCT_COPY_FILES += \
	device/nexell/drone2/audio/audio_policy.conf:system/etc/audio_policy.conf

PRODUCT_PACKAGES += \
        HdmiDualPlayback

################################################################################
# media, camera
################################################################################
PRODUCT_COPY_FILES += \
	device/nexell/drone2/media_codecs.xml:system/etc/media_codecs.xml \
	device/nexell/drone2/media_profiles.xml:system/etc/media_profiles.xml

################################################################################


# busybox
################################################################################
PRODUCT_COPY_FILES += \
    device/nexell/drone2/tools/busybox:system/bin/busybox

################################################################################

# sensor
################################################################################
PRODUCT_PACKAGES += \
	sensors.drone2

################################################################################
# camera
################################################################################
PRODUCT_PACKAGES += \
	camera.slsiap

#add by cym 20161009
PRODUCT_PACKAGES += \
        UvcWebCam
#end add

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
EN_FFMPEG_EXTRACTOR := true
EN_FFMPEG_AUDIO_DEC := true
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
  device/nexell/drone2/tablet_core_hardware.xml:system/etc/permissions/tablet_core_hardware.xml \
  frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml \
  frameworks/native/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
  frameworks/native/data/etc/android.hardware.wifi.direct.xml:system/etc/permissions/android.hardware.wifi.direct.xml \
  frameworks/native/data/etc/android.hardware.camera.flash-autofocus.xml:system/etc/permissions/android.hardware.camera.flash-autofocus.xml \
  frameworks/native/data/etc/android.hardware.camera.front.xml:system/etc/permissions/android.hardware.camera.front.xml \
  frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml \
  frameworks/native/data/etc/android.hardware.usb.host.xml:system/etc/permissions/android.hardware.usb.host.xml \
  frameworks/native/data/etc/android.hardware.sensor.accelerometer.xml:system/etc/permissions/android.hardware.sensor.accelerometer.xml \
  frameworks/native/data/etc/android.hardware.audio.low_latency.xml:system/etc/permissions/android.hardware.audio.low_latency.xml \
  frameworks/native/data/etc/android.hardware.bluetooth.xml:system/etc/permissions/android.hardware.bluetooth.xml \
  frameworks/native/data/etc/android.hardware.bluetooth_le.xml:system/etc/permissions/android.hardware.bluetooth_le.xml \
  linux/platform/s5p4418/library/lib/ratecontrol/libnxvidrc_android.so:system/lib/libnxvidrc_android.so

PRODUCT_AAPT_CONFIG := xlarge hdpi xhdpi large
PRODUCT_AAPT_PREF_CONFIG := xhdpi

PRODUCT_CHARACTERISTICS := tablet,usbstorage

DEVICE_PACKAGE_OVERLAYS := \
	device/nexell/drone2/overlay

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

# Product Property
# common
PRODUCT_PROPERTY_OVERRIDES := \
	wifi.interface=wlan0 \
	ro.sf.lcd_density=160

# openl ui property
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

#MediaTek mt66xx 
#ifeq ($(strip $(BOARD_CONNECTIVITY_VENDOR)), MediaTek)

#ifeq ($(strip $(BOARD_CONNECTIVITY_MODULE)), combo_mt66xx)
#$(call inherit-product, hardware/mediatek/config/$(strip $(BOARD_CONNECTIVITY_MODULE))/product_package.mk)
$(call inherit-product, hardware/mediatek/config/combo_mt66xx/product_package.mk)
#endif

#mt6620 wifi drivers and config file.
PRODUCT_COPY_FILES += \
	kernel/drivers/misc/mediatek/combo_mt66xx/wmt/mtk_hif_sdio.ko:system/lib/modules/mtk_hif_sdio.ko \
	kernel/drivers/misc/mediatek/combo_mt66xx/wmt/mtk_stp_uart.ko:system/lib/modules/mtk_stp_uart.ko \
	kernel/drivers/misc/mediatek/combo_mt66xx/wmt/mtk_stp_wmt.ko:system/lib/modules/mtk_stp_wmt.ko \
	kernel/drivers/misc/mediatek/combo_mt66xx/wmt/mtk_wmt_wifi.ko:system/lib/modules/mtk_wmt_wifi.ko \
	kernel/drivers/net/wireless/combo_mt66xx/mt6620/wlan/wlan_mt6620.ko:system/lib/modules/wlan_mt6620.ko \
	kernel/drivers/misc/mediatek/combo_mt66xx/wmt/mtk_stp_gps.ko:system/lib/modules/mtk_stp_gps.ko \
	kernel/drivers/misc/mediatek/combo_mt66xx/wmt/mtk_stp_bt.ko:system/lib/modules/mtk_stp_bt.ko \
	hardware/mediatek/config/combo_mt66xx/init.combo_mt66xx.rc:root/init.connectivity.rc 
#endif

#Install ESFileExplorer
PRODUCT_COPY_FILES += \
        device/nexell/drone2/apk/ESFileExplorer/ESFileExplorer.apk:system/app/ESFileExplorer.apk
PRODUCT_COPY_FILES += \
        device/nexell/drone2/apk/ESFileExplorer/libmyaes.so:system/lib/libmyaes.so
#end Install

#Install Chess
PRODUCT_COPY_FILES += \
        device/nexell/drone2/apk/chess/chess_1.62.apk:system/app/chess_1.62.apk
PRODUCT_COPY_FILES += \
        device/nexell/drone2/apk/chess/libxiangqi.so:system/lib/libxiangqi.so
#end Install

#Install start_super
PRODUCT_COPY_FILES += \
        device/nexell/drone2/apk/start_super/start_super_1.8.apk:system/app/start_super_1.8.apk
PRODUCT_COPY_FILES += \
        device/nexell/drone2/apk/start_super/libbspatch.so:system/lib/libbspatch.so
PRODUCT_COPY_FILES += \
        device/nexell/drone2/apk/start_super/libcocos2dx-game.so:system/lib/libcocos2dx-game.so
PRODUCT_COPY_FILES += \
        device/nexell/drone2/apk/start_super/libprotectClass.so:system/lib/libprotectClass.so
#end Install

#Install 2048
PRODUCT_COPY_FILES += \
        device/nexell/drone2/apk/2048/2048.apk:system/app/2048.apk
PRODUCT_COPY_FILES += \
        device/nexell/drone2/apk/2048/libappJni.so:system/lib/libappJni.so
PRODUCT_COPY_FILES += \
        device/nexell/drone2/apk/2048/libbase64encoder_v1_4.so:system/lib/libbase64encoder_v1_4.so
PRODUCT_COPY_FILES += \
        device/nexell/drone2/apk/2048/libcocos2dcpp.so:system/lib/libcocos2dcpp.so
#end Install

#Install xunleikankan
PRODUCT_COPY_FILES += \
        device/nexell/drone2/apk/xunleikankan/xunleikankan.apk:system/app/xunleikankan.apk
PRODUCT_COPY_FILES += \
        device/nexell/drone2/apk/xunleikankan/libBaiduMapSDK_v2_3_0.so:system/lib/libBaiduMapSDK_v2_3_0.so
PRODUCT_COPY_FILES += \
        device/nexell/drone2/apk/xunleikankan/libjpush.so:system/lib/libjpush.so
PRODUCT_COPY_FILES += \
        device/nexell/drone2/apk/xunleikankan/liblocSDK4.so:system/lib/liblocSDK4.so
PRODUCT_COPY_FILES += \
        device/nexell/drone2/apk/xunleikankan/libmediaplayer_jni.so:system/lib/libmediaplayer_jni.so
PRODUCT_COPY_FILES += \
        device/nexell/drone2/apk/xunleikankan/libmediaserver_jni.so:system/lib/libmediaserver_jni.so
PRODUCT_COPY_FILES += \
        device/nexell/drone2/apk/xunleikankan/libMMANDKSignature.so:system/lib/libMMANDKSignature.so
#end Install

#Install GPS TEST
PRODUCT_COPY_FILES += \
        device/nexell/drone2/apk/gps_test/GPS_TEST_1.3.2.apk:system/app/GPS_TEST_1.3.2.apk
#PRODUCT_COPY_FILES += \
        device/nexell/drone2/apk/gps_test/AndroiTS_1.46.apk:system/app/AndroiTS_1.46.apk

#Install HdmiDualPlayback
PRODUCT_COPY_FILES += \
        device/nexell/drone2/apk/HdmiDualPlayback.apk:system/app/HdmiDualPlayback.apk
#end Install

# wifi
ifeq ($(BOARD_WIFI_VENDOR),realtek)
$(call inherit-product-if-exists, hardware/realtek/wlan/config/p2p_supplicant.mk)
endif

ifeq ($(BOARD_WIFI_VENDOR),broadcom)
WIFI_BAND := 802_11_BG
$(call inherit-product-if-exists, hardware/broadcom/wlan/bcmdhd/firmware/bcm4329/device-bcm.mk)
endif

# slsiap
$(call inherit-product-if-exists, hardware/samsung_slsi/slsiap/slsiap.mk)

# google gms
#$(call inherit-product-if-exists, vendor/google/gapps/device-partial.mk)
