ifndef	TOPDIR
TOPDIR	:= ../..
endif

########################################################################
# Get Linux Build Enviornment:
include ../../../../build.env

########################################################################
# Module Definition & Version Management
BUILD_MOD_NAME		:= DVR_MANAGER

BUILD_NUMBER_MAJOR	:= 1
BUILD_NUMBER_MINOR	:= 0

BUILD_REVISION_FILE	:= build_revision_number
BUILD_INFO_FILE		:= NX_DvrManagerBuildInfo.h

######################################################################
# Build options

# Prevent: "warning: creating a DT_TEXTREL in object"
CFLAGS		+= -fpic -DARM_11
CPPFLAGS	+= -fpic -DARM_11

INCLUDE		+= -I./
INCLUDE		+= -I$(LIBSDIR)/include
INCLUDE		+= -I$(TOPDIR)/include
INCLUDE		+= -I../libnxfilters
INCLUDE 	+= -I$(TOPDIR)/include/live555/liveMedia
INCLUDE 	+= -I$(TOPDIR)/include/live555/groupsock
INCLUDE 	+= -I$(TOPDIR)/include/live555/BasicUsageEnvironment
INCLUDE 	+= -I$(TOPDIR)/include/live555/UsageEnvironment

LIBRARY		+= -L$(LIBSDIR)/lib
LIBRARY		+= -L$(TOPDIR)/lib
LIBRARY		+= -L../libnxfilters
LIBRARY 	+= -L$(TOPDIR)/lib/live555

LIBRARY		+= -lnxfilters
LIBRARY		+= -lnxaacenc -lnxmp3enc
LIBRARY		+= -lnxmp4mux -lnxtsmux
LIBRARY 	+= -lnxvmem -lnxvip -lnxdsp -lnxv4l2 -lnxvpu
LIBRARY 	+= -lliveMedia -lgroupsock -lBasicUsageEnvironment -lUsageEnvironment
LIBRARY		+= -lstdc++ -lpthread -lasound

######################################################################
# Target
COBJS  	:= 
CPPOBJS	:= CNX_DvrNotify.o CNX_DvrManager.o NX_DvrControl.o

LIBNAME := libnxdvr

SO_NAME	:= $(LIBNAME).so
TARGET	:= $(LIBNAME).so

######################################################################
# Build
OBJS	:= $(COBJS) $(CPPOBJS)

all: $(TARGET)

$(TARGET): depend $(OBJS)
	$(CC) $(LDFLAGS) -shared -Wl,-soname,$(SO_NAME) -o $@ $(OBJS) $(LIBRARY)
	chmod 644 $(TARGET)

clean:
	@find $(OBJTREE) -type f \
		\( -name '*.o'	-o -name $(TARGET) -o -name '*.a' -o -name '*.so.*' -o \
		-name '*.bin' -o -name '*.map' -o -name 'core' -o -name '*~' -o -name '.depend' \) \
		-print | xargs rm -f

install:
	cp -a $(SO_NAME) $(TOPDIR)/lib
	cp -a NX_DvrControl.h $(TOPDIR)/include

distclean:
	rm -f $(TOPDIR)/lib/$(SO_NAME)
	rm -f $(TOPDIR)/include/NX_DvrControl.h
	@find $(OBJTREE) -type f \
		\( -name '*.o'	-o -name $(TARGET) -o -name '*.a' -o -name '*.so.*' -o \
		-name '*.bin' -o -name '*.map' -o -name 'core' -o -name '*~' -o -name '.depend' \) \
		-print | xargs rm -f

update:
	@echo $$(($$(cat $(BUILD_REVISION_FILE).txt) + 1)) > $(BUILD_REVISION_FILE).txt
	@echo
	@echo "Update Build Number: $(BUILD_MOD_NAME) version $(BUILD_NUMBER_MAJOR).$(BUILD_NUMBER_MINOR).$$(cat $(BUILD_REVISION_FILE).txt) (built by $$(whoami) at $$(date) )"
	@echo
	@echo "#define $(BUILD_MOD_NAME)_MAJOR		$(BUILD_NUMBER_MAJOR)			// Build Major Number" > $(BUILD_INFO_FILE)
	@echo "#define $(BUILD_MOD_NAME)_MINOR		$(BUILD_NUMBER_MINOR)			// Build Minor Number" >> $(BUILD_INFO_FILE)
	@echo "#define $(BUILD_MOD_NAME)_REVISION	$$(cat $(BUILD_REVISION_FILE).txt)			// Build Revision Number" >> $(BUILD_INFO_FILE)
	@echo "#define $(BUILD_MOD_NAME)_DATE		__DATE__	// Build Date" >> $(BUILD_INFO_FILE)
	@echo "#define $(BUILD_MOD_NAME)_TIME		__TIME__	// Build Time" >> $(BUILD_INFO_FILE)
	@echo "#define $(BUILD_MOD_NAME)_USER		\"$$(whoami)\"	// Build User" >> $(BUILD_INFO_FILE)


#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp)
INCS := $(INCLUDE)
depend dep:
	$(CC)  -M  $(CPPFLAGS)   $(INCS) $(SRCS) > .depend
