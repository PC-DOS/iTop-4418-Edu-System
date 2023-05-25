########################################################################
# Get Linux Build Enviornment:
ifndef TOPDIR
TOPDIR	:= ../../..
endif

include $(TOPDIR)/build.env

LIB_INSTALL	:=	../../lib
INC_INSTALL	:= 	../../include


######################################################################
# Build options
INCLUDE += -I./
INCLUDE += -I./ -I$(TOPDIR)/inc

LIBRARY	+= -lstdc++ -lpthread -lgobject-2.0 -lm
LIBRARY += -L$(GLIB_LIB) -lglib-2.0 

LIBRARY += -L./theoraparser -ltheoraparser
######################################################################
# Target
COBJS  	:= 
COBJS  	+= 

CPPOBJS	 = NX_CManager.o NX_DebugMsg.o NX_SystemCall.o
CPPOBJS	+= NX_MoviePlay.o 


CFLAGS		+= -fPIC -g
CPPFLAGS	+= -fPIC -g

LIBNAME	:= libnxfiltermanager
TARGET	:= $(LIBNAME).so

######################################################################
# Build
OBJS	:= $(COBJS) $(CPPOBJS)

all: $(TARGET)

$(TARGET):	depend $(OBJS)
	$(AR) $(ARFLAGS) $(LIBNAME).a $(OBJS)
	$(CC) $(LDFLAGS) -shared -Wl,-soname,$(TARGET) -o $@ $(OBJS) $(LIBRARY)

install:
	install -m 755 -d $(LIB_INSTALL)
	install -m 644 $(LIBNAME).a $(LIB_INSTALL)
	install -m 644 $(TARGET) $(LIB_INSTALL)

clean:
	@find $(OBJTREE) -type f \
		\( -name '*.o'	-o -name $(LIBNAME).a -o -name '*.map' \
		-o -name '.depend' \) -print | xargs rm -f

distclean: clean
	rm -f $(LIB_INSTALL)/$(LIBNAME).a
	rm -f $(LIB_INSTALL)/$(TARGET)

#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp)
INCS := $(INCLUDE)
depend dep:
	$(CC)  -M  $(CPPFLAGS)   $(INCS) $(SRCS) > .depend
