TARGET			= control_3g

KDIR			:= ../../../../kernel/kernel-3.4.39
#STRIP_CMD		= arm-cortex_a9-linux-gnueabi-strip

obj-m			+= control_3g.o


################################################################################
#	Build options for board type
################################################################################
PWD     := $(shell pwd)
KBUILD_CFLAGS += -I./include
#KBUILD_CFLAGS += -I./firmware
#KBUILD_CFLAGS += -I../libnxvmalloc

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	@find . -type f \( -name '*.ko' -o -name '*.mod.*'		\
					-o -name '*.o' -o -name '.*.cmd'		\
					-o -name '*.order' -o -name '*.symvers'	\
					-o -name '*.' \) -print | xargs rm -f
	rm -rf .tmp_versions

