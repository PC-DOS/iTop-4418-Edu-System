ifndef	JOBS
JOBJS	:= 8
endif

DIR		:= 
######################################################################
# Build options

# Multi Media Libraries
DIR += libion
DIR += libnxmalloc
DIR += libnxv4l2
DIR += libnxvpu

# Movie Player for FFMPEG
DIR += libnxfilter
DIR += libnxfilterhelp
DIR += libnxfiltermanager

# Movie Player for FFMPEG
DIR += libnxtypefind
DIR += libnxmovieplayer

# Graphic library
DIR += libnxscaler
DIR += libnxgraphictools

# I/O Wrapper
DIR += libcec
DIR += libnxadc
DIR += libnxgpio
DIR += libnxaudio
DIR += libnxpwm
DIR += libnxiso7816

# Etc Libraries
DIR += libnxuevent
DIR += libnxnmeaparser

######################################################################
# Build
all:
	@for dir in $(DIR); do			\
	make -C $$dir || exit $?;		\
	make -C $$dir install; 			\
	done

install:
	@for dir in $(DIR); do			\
	make -C $$dir install || exit $?;	\
	done

clean:
	@for dir in $(DIR); do			\
	make -C $$dir clean || exit $?;	\
	done

distclean:
	@for dir in $(DIR); do			\
	make -C $$dir distclean || exit $?;	\
	done
