########################################################################
# Get Linux Build Enviornment:
include ../../../build.env

LIB_INSTALL	:=	../../lib
INC_INSTALL	:= 	../../include

######################################################################
# Build options
INCLUDE += -I./ -I../../include -I$(KERNDIR)/include

LIBRARY += -L../../lib/ -L../../lib/ratecontrol/
LIBRARY += -lnxvmem
LIBRARY += -lnxvidrc

######################################################################
# Target
COBJS  	:= 
COBJS  	+= nx_video_api.o
CPPOBJS	:=  

CFLAGS	+= -g

LIBNAME	:= libnxvpu
TARGET  := libnxvpu.so

######################################################################
# Build
OBJS	:= $(COBJS) $(CPPOBJS)

all: $(TARGET)

$(TARGET):	depend $(OBJS)
	$(AR) $(ARFLAGS) $(LIBNAME).a $(OBJS)
	$(CC) $(LDFLAGS) -shared -Wl,-soname,$(TARGET) -o $@ $(OBJS) $(LIBRARY)

install:
	install -m 755 -d $(LIB_INSTALL)
	install -m 644 $(TARGET) $(LIB_INSTALL)
	install -m 644 $(LIBNAME).a $(LIB_INSTALL)

clean:
	rm -f *.o *.so *.a .depend

distclean: clean
	rm -f $(LIB_INSTALL)/$(TARGET)
	rm -f $(LIB_INSTALL)/$(LIBNAME).a


#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp) $(APPOBJS:.o=.c)
INCS := $(INCLUDE)
depend dep:
	$(CC) -M $(CFLAGS) $(INCS) $(SRCS) > .depend
