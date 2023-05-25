ifndef TOPDIR
TOPDIR	:=	../..
endif
########################################################################
# Get Linux Build Enviornment:
include ../../../../build.env

######################################################################
# Build options
INCLUDE += -I./ -I$(TOPDIR)/include
INCLUDE += -I$(LIBSDIR)/include

INCLUDE += -I$(TOPDIR)/include/live555/liveMedia
INCLUDE += -I$(TOPDIR)/include/live555/groupsock
INCLUDE += -I$(TOPDIR)/include/live555/BasicUsageEnvironment
INCLUDE += -I$(TOPDIR)/include/live555/UsageEnvironment

LIBRARY	+= -L$(TOPDIR)/lib
LIBRARY += -L$(LIBSDIR)/lib -L$(LIBSDIR)/lib/ratecontrol
LIBRARY += -lnxrtp
LIBRARY += -lnxvmem -lnxvip -lnxdsp -lnxv4l2 -lnxvpu -lnxvidrc
LIBRARY	+= -lstdc++ -lpthread -lasound

LIBRARY += -I$(TOPDIR)/lib/live555/liveMedia
LIBRARY += -I$(TOPDIR)/lib/live555/groupsock
LIBRARY += -I$(TOPDIR)/lib/live555/BasicUsageEnvironment
LIBRARY += -I$(TOPDIR)/lib/live555/UsageEnvironment


######################################################################
# Target
CPPOBJS	+= NX_RtpMain.o

TARGET 	:= nxrtpsol
CPPFLAGS += -g

######################################################################
# Build
OBJS	:= $(COBJS) $(CPPOBJS)

all: $(TARGET)

$(TARGET): depend $(OBJS)
	$(CC) $(CPPFLAGS) $(OBJS) -o $@ $(LIBRARY)

clean:
	@find $(OBJTREE) -type f \
		\( -name '*.o'	-o -name $(TARGET) -o -name '*.map' -o -name '.depend' \) -print \
		| xargs rm -f

install:
	cp -a $(TARGET) $(TOPDIR)/bin

distclean:
	rm -f $(TOPDIR)/bin/$(TARGET)
	@find $(OBJTREE) -type f \
		\( -name '*.o'	-o -name $(TARGET) -o -name '*.map' -o -name '.depend' \) -print \
		| xargs rm -f


#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp)
INCS := $(INCLUDE)
depend dep:
	$(CC)  -M  $(CFLAGS)   $(INCS) $(SRCS) > .depend
