ifndef	TOPDIR
TOPDIR	:= ../..
endif
########################################################################
# Get Linux Build Enviornment:
include ../../../../build.env

######################################################################
# Build options

# Prevent: "warning: creating a DT_TEXTREL in object"
CFLAGS		+= -fpic -DARM_11
CPPFLAGS	+= -fpic -DARM_11

INCLUDE		+= -I./
INCLUDE		+= -I$(LIBSDIR)/include
INCLUDE		+= -I$(TOPDIR)/include
INCLUDE		+= -I../libnxfilters

LIBRARY		+= -L$(LIBSDIR)/lib
LIBRARY		+= -L$(TOPDIR)/lib
LIBRARY		+= -L../libnxfilters/ -lnxfilters
LIBRARY		+= -lnxaacenc
LIBRARY		+= -lnxmp4mux
LIBRARY 	+= -lnxvmem -lnxvip -lnxdsp -lnxv4l2 -lnxvpu
LIBRARY		+= -lstdc++ -lpthread -lasound

######################################################################
# Target
COBJS  	:= 
CPPOBJS	:= CNX_Mp4Manager.o CNX_Mp4Notify.o

LIBNAME := libnxmp4manager

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
	cp -a INX_Mp4Manager.h $(TOPDIR)/include

distclean:
	rm -f $(TOPDIR)/lib/$(SO_NAME)
	rm -f $(TOPDIR)/include/INX_Mp4Manager.h
	@find $(OBJTREE) -type f \
		\( -name '*.o'	-o -name $(TARGET) -o -name '*.a' -o -name '*.so.*' -o \
		-name '*.bin' -o -name '*.map' -o -name 'core' -o -name '*~' -o -name '.depend' \) \
		-print | xargs rm -f

#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp)
INCS := $(INCLUDE)
depend dep:
	$(CC)  -M  $(CPPFLAGS)   $(INCS) $(SRCS) > .depend
