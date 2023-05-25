ifndef	TOPDIR
TOPDIR	:= ../../..
endif

ifndef	JOBS
JOBS	:= 8
endif

# Get Linux Build Enviornment
include $(TOPDIR)/build.env

######################################################################
# Build options
SUB_DIRS	:= nxdvrmonitor nxdvrsol nxguisol nxhlssol nxmp4encsol nxrtpsol nxtranscodingsol

all: compile_subdirs 

compile_subdirs:
	@for dir in $(SUB_DIRS); do \
	make -C $$dir -j$(JOBS) || exit $?;	\
	done

clean:
	@for dir in $(SUB_DIRS); do \
	make -C $$dir clean;		\
	done

install:
	@for dir in $(SUB_DIRS); do	\
	make -C $$dir install;		\
	done

distclean:
	@for dir in $(SUB_DIRS); do	\
	make -C $$dir distclean;		\
	done
