ifndef TOPDIR
TOPDIR	:= ../..
endif

########################################################################
# Get Linux Build Enviornment:
include $(TOPDIR)/build.env
include $(TOPDIR)/config.mak

######################################################################
# Build options

# Prevent: "warning: creating a DT_TEXTREL in object"
CFLAGS		+= -fpic
CPPFLAGS	+= -fpic
INCLUDE 	+= -I./ -I$(TOPDIR)/inc -I$(SDL_INC)
LIBRARY		+= -lSDL -lSDL_image
######################################################################
# Target
COBJS  	:= 
CPPOBJS	:= sensor_test.o 

LIBNAME := libnxtestsensor

SO_NAME	:= $(LIBNAME).so
TARGET	:= $(LIBNAME).so
######################################################################
# Build
OBJS	:= $(COBJS) $(CPPOBJS)

all: $(TARGET)

$(TARGET): depend $(OBJS)
	$(CC) $(LDFLAGS) -shared -Wl,-soname,$(SO_NAME) -o $@ $(OBJS) $(LIBRARY)
	install -m 755 $(TARGET) $(TOPDIR)/libs/

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
