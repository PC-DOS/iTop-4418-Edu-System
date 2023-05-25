ifndef	JOBS
JOBJS	:= 8
endif

DIR		:= 
######################################################################
# Build options

# Tested Features
DIR += common 
DIR += lcd_test 
DIR += touch_test 
DIR += audio_rec_test 
DIR += backlight
DIR += key_test 
DIR += vibrator
DIR += camera_test
DIR += hdmi_test
DIR += wifi_test 
DIR	+= sd_test 
DIR	+= usb_test 
DIR	+= main

######################################################################
# Build
all:
	@for dir in $(DIR); do		\
	make -C $$dir -j$(JOBS) || exit $?;	\
	done

clean:
	@for dir in $(DIR); do		\
	make -C $$dir clean;		\
	done

