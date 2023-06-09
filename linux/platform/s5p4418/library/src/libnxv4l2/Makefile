######################################################################
# Get Linux Build Enviornment:
include ../../../build.env

LIB_INSTALL	:=	../../lib
INC_INSTALL	:= 	../../include

######################################################################
# Build Options
CFLAGS		:= -O2 -g -fpic
CPPFLAGS	:= -O2 -g -fpic
INCLUDE		+= -I./ -I$(KERNDIR)/include
LIBRARY		+= -L./
LIBRARY		+= -lnxv4l2 -lstdc++

######################################################################
# Target
# nxp-v4l2.cpp 			: v4l2 original libaray
# nxp-v4l2-media.cpp	: v4l2 libaray for media-solution team
COBJS			:= nx_vip.o nx_dsp.o
CPPOBJS			:= nxp-v4l2.o nxp-v4l2-media.o nxp-v4l2-dev.o

NXV4L2_OBJS		:= nxp-v4l2.o nxp-v4l2-dev.o
NXV4L2_LIBNAME	:= libv4l2-nexell
NXV4L2_TARGET	:= $(NXV4L2_LIBNAME).so

V4L2_OBJS		:= nxp-v4l2-media.o nxp-v4l2-dev.o
V4L2_LIBNAME	:= libnxv4l2
V4L2_TARGET		:= $(V4L2_LIBNAME).so

VIP_OBJS		:= nx_vip.o
VIP_LIBNAME		:= libnxvip
VIP_TARGET		:= $(VIP_LIBNAME).so

DSP_OBJS		:= nx_dsp.o
DSP_LIBNAME		:= libnxdsp
DSP_TARGET		:= $(DSP_LIBNAME).so

######################################################################
# Build
OBJS			:= $(COBJS) $(CPPOBJS)

all: $(NXV4L2_TARGET) $(V4L2_TARGET) $(VIP_TARGET) $(DSP_TARGET)

$(NXV4L2_TARGET): depend $(NXV4L2_OBJS)
	$(CC) $(LDFLAGS) -shared -Wl,-soname,$(NXV4L2_TARGET) -o $@ $(NXV4L2_OBJS)
#	$(AR) $(ARFLAGS) $(NXV4L2_LIBNAME).a $(NXV4L2_OBJS)

$(V4L2_TARGET):	depend $(V4L2_OBJS)
	$(CC) $(LDFLAGS) -shared -Wl,-soname,$(V4L2_TARGET) -o $@ $(V4L2_OBJS)
#	$(AR) $(ARFLAGS) $(V4L2_LIBNAME).a $(V4L2_OBJS)

$(VIP_TARGET): depend $(VIP_OBJS)
	$(CC) $(LDFLAGS) -shared -Wl,-soname,$(VIP_TARGET) -o $@ $(VIP_OBJS) $(LIBRARY)
#	$(AR) $(ARFLAGS) $(VIP_LIBNAME).a $(VIP_OBJS)

$(DSP_TARGET): depend $(DSP_OBJS)
	$(CC) $(LDFLAGS) -shared -Wl,-soname,$(DSP_TARGET) -o $@ $(DSP_OBJS) $(LIBRARY)
#	$(AR) $(ARFLAGS) $(DSP_LIBNAME).a $(DSP_OBJS)

clean:
	rm -f *.o *.a *.so *.so.* .depend

install:
	install -m 755 -d $(LIB_INSTALL)
	install -m 644 $(NXV4L2_TARGET) $(LIB_INSTALL)
	install -m 644 $(V4L2_TARGET) $(LIB_INSTALL)
	install -m 644 $(VIP_TARGET) $(LIB_INSTALL)
	install -m 644 $(DSP_TARGET) $(LIB_INSTALL)

distclean: clean
	rm -f $(LIB_INSTALL)/$(NXV4L2_TARGET)
	rm -f $(LIB_INSTALL)/$(V4L2_TARGET)
	rm -f $(LIB_INSTALL)/$(VIP_TARGET)
	rm -f $(LIB_INSTALL)/$(DSP_TARGET)

#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp)
INCS := $(INCLUDE)
depend dep:
	$(CC)  -M  $(CFLAGS)   $(INCS) $(SRCS) > .depend
