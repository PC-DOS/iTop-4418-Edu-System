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

LIBRARY	+= -L$(TOPDIR)/lib
LIBRARY += -L$(LIBSDIR)/lib -L$(LIBSDIR)/lib/ratecontrol
LIBRARY += -lnxgraphictools -lEGL -lGLESv1_CM -lGLESv2 -lVR
LIBRARY += -lnxdvr -lnxnmeaparser -lnxaudio -lnxgpio -lnxadc
LIBRARY += -lion -lnxvmem -lnxvip -lnxdsp -lnxv4l2 -lnxvpu -lnxvidrc
LIBRARY	+= -lstdc++ -lpthread -lasound

######################################################################
# Target
COBJS	+= NX_Queue.o NX_Semaphore.o NX_DvrCmdQueue.o NX_DvrTools.o
COBJS	+= NX_DvrLedCtrl.o
COBJS	+= NX_DvrFileManager.o NX_DvrFileQueue.o
COBJS	+= NX_DvrGpsManager.o
COBJS	+= NX_DvrGsensorManager.o NX_DvrGsensor.o
COBJS	+= NX_DvrPowerManager.o
COBJS	+= NX_DvrMain.o

TARGET 	:= nxdvrsol
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
