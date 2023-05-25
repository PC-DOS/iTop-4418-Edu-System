ifndef TOPDIR
TOPDIR	:= ../../../../
endif

########################################################################
# Get Linux Build Enviornment:
include $(TOPDIR)/build.env

######################################################################
# Build options

# Prevent: "warning: creating a DT_TEXTREL in object"
CFLAGS		+= -fpic
CPPFLAGS	+= -fpic
INCLUDE 	+= -I./ -I../../inc

######################################################################
# Target
COBJS  	:= 
CPPOBJS	:= lcd_test.o 

LIBNAME := libnxtestLcd

SO_NAME	:= $(LIBNAME).so
TARGET	:= $(LIBNAME).so
######################################################################
# Build
OBJS	:= $(COBJS) $(CPPOBJS)

all: $(TARGET)

$(TARGET): depend $(OBJS)
	$(CC) $(LDFLAGS) -shared -Wl,-soname,$(SO_NAME) -o $@ $(OBJS) $(LIBRARY)
	install -m 755 $(TARGET) ../../libs/

clean:
	rm -f *.a *.o .depend $(TARGET)

#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp)
INCS := $(INCLUDE)
depend dep:
	$(CC)  -M  $(CPPFLAGS)   $(INCS) $(SRCS) > .depend
