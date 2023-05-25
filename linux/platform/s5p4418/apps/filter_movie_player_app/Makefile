########################################################################
# Get Linux Build Enviornment:
ifndef TOPDIR
TOPDIR	:= ../..
endif
include $(TOPDIR)/build.env

######################################################################
# Build options
INCLUDE += -I./ -I$(LIBSDIR)/include
LIBRARY	+= -lstdc++ -lpthread -lgobject-2.0 -lm
LIBRARY += -L$(LIBSDIR)/lib -lnxfilter -lnxfilterhelp -lnxfiltermanager


INCLUDE += -I./
INCLUDE += -I../../library/include
LIBRARY	+= -L../../library/lib -L../../library/lib/ratecontrol
LIBRARY += -lnxvpu -lnxdsp -lnxvip -lnxv4l2 -lnxvmem  -lnxvidrc

LIBRARY += -L$(LIBSDIR)/lib -lnxuevent

LIBRARY	+= -lasound  -lavcodec -lavformat -lstdc++


######################################################################
# Target
TARGET	:= filter_movie_player_app

######################################################################
# Build
COBJS  	:= main.o
OBJS	:= $(COBJS) $(CPPOBJS)

all: $(TARGET) 

clean:
	rm -f $(TARGET) *.o .depend

$(TARGET): depend $(OBJS)
	$(CC) $(CPPFLAGS) $(OBJS) -o $(TARGET) $(LIBRARY)
	@echo "done"

#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp)
INCS := $(INCLUDE)
depend dep:
	$(CC)  -M  $(CFLAGS)   $(INCS) $(SRCS) > .depend
