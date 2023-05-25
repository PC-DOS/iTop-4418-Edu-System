ifeq ($(TARGET_CPU_VARIANT2),s5p4418)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
# LOCAL_PRELINK_MODULE := false

LOCAL_SHARED_LIBRARIES :=	\
	liblog \
	libcutils

LOCAL_C_INCLUDES += system/core/include/ion \
					$(TOP)/hardware/samsung_slsi/slsiap/include \
					$(TOP)/linux/platform/s5p4418/library/include

LOCAL_SRC_FILES := \
	nx_alloc_mem_ion.c

LOCAL_MODULE := libnxmalloc

LOCAL_MODULE_TAGS := optional

include $(BUILD_STATIC_LIBRARY)

endif
