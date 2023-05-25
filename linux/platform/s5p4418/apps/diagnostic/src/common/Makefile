ifndef TOPDIR
TOPDIR	:= ../../../..
endif

########################################################################
# Get Linux Build Enviornment:
include $(TOPDIR)/build.env

######################################################################
# Build options
INCLUDE += -I./ -I../../inc

LIBRARY	+= -lstdc++ -lSDL -lSDL_ttf

######################################################################
# Target
COBJS  	:= 
# add sdal test module
CPPOBJS	:= utils.o

TARGET 	:= libnxdiagcommon.so
CPPFLAGS += -g

######################################################################
# Build
OBJS	:= $(COBJS) $(CPPOBJS)

all: $(TARGET)

$(TARGET): depend $(OBJS)
	$(CC) $(LDFLAGS) -shared -Wl,-soname,$(SO_NAME) -o $@ $(OBJS) $(LIBRARY)
	install -m 755 $(TARGET) ../../libs

clean:
	rm -f *.o .depend $(TARGET)
#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp)
INCS := $(INCLUDE)
depend dep:
	$(CC)  -M  $(CPPFLAGS)   $(INCS) $(SRCS) > .depend
