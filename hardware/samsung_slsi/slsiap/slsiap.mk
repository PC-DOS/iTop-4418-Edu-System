# hw composer HAL
PRODUCT_PACKAGES += \
	hwcomposer.slsiap

# gralloc HAL
PRODUCT_PACKAGES += \
	gralloc.slsiap

# audio HAL
PRODUCT_PACKAGES += \
	audio.primary.slsiap

# lights HAL
PRODUCT_PACKAGES += \
	lights.slsiap

# power HAL
PRODUCT_PACKAGES += \
	power.slsiap

# keystore HAL
PRODUCT_PACKAGES += \
	keystore.slsiap

# gps HAL
PRODUCT_PACKAGES += \
        gps.slsiap

# ion library
PRODUCT_PACKAGES += \
	libion-nexell

# v4l2 library
PRODUCT_PACKAGES += \
	libv4l2-nexell

# omx
PRODUCT_PACKAGES += \
	libstagefrighthw \
	libnx_vpu \
	libNX_OMX_VIDEO_DECODER \
	libNX_OMX_VIDEO_ENCODER \
	libNX_OMX_Base \
	libNX_OMX_Core \
	libNX_OMX_Common


# stagefright FFMPEG compnents
ifeq ($(EN_FFMPEG_AUDIO_DEC),true)
PRODUCT_PACKAGES += libNX_OMX_AUDIO_DECODER_FFMPEG
endif

ifeq ($(EN_FFMPEG_EXTRACTOR),true)
PRODUCT_PACKAGES += libNX_FFMpegExtractor
endif

# for ogl
#PRODUCT_PACKAGES += \
	#libVR \
	#libGLESv1_CM_vr \
	#libGLESv2_vr \
	#libEGL_vr

# haptic HAL
ifeq ($(BOARD_HAS_HAPTIC),true)
PRODUCT_PACKAGES += \
	libhapticapi \
	libhapticjavaapi
endif

PRODUCT_VENDOR_KERNEL_HEADERS := hardware/samsung_slsi/slsiap/kernel-headers
