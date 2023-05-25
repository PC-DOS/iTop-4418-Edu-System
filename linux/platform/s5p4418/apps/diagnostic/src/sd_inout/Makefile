########################################################################
# Get Linux Build Enviornment:
include ../../../../build.env
include ../config.mak
######################################################################
# Build options

# Prevent: "warning: creating a DT_TEXTREL in object"
CFLAGS		+= -fpic
CPPFLAGS	+= -fpic
INCLUDE 	+= -I./ -I../inc -I$(SDL_INC)

######################################################################
# Target
COBJS  	:= 
CPPOBJS	:= sd_inout.o 

LIBNAME := libnxtestsdin

SO_NAME	:= $(LIBNAME).so
TARGET	:= $(LIBNAME).so
######################################################################
# Build
OBJS	:= $(COBJS) $(CPPOBJS)

all: $(TARGET)

$(TARGET): depend $(OBJS)
	$(CC) $(LDFLAGS) -shared -Wl,-soname,$(SO_NAME) -o $@ $(OBJS) $(LIBRARY)
	$(AR) $(ARFLAGS) $(LIBNAME).a $(OBJS)
	chmod 644 $(TARGET)
	chmod 644 $(LIBNAME).a

install: $(TARGET)
	install -m 755 -d $(LIB_INSTALL)
	install -m 644 $(LIBNAME).a $(LIB_INSTALL)
	install -m 644 $^ $(LIB_INSTALL)

clean:
	@find $(OBJTREE) -type f \
		\( -name '*.o'	-o -name $(TARGET) -o -name '*.a' -o -name '*.so.*' -o \
		-name '*.bin' -o -name '*.map' -o -name 'core' -o -name '*~' -o -name '.depend' \) \
		-print | xargs rm -f
	rm -f ../libs/$(TARGET)
cleanall: clean
	rm -f $(LIB_INSTALL)/$(LIBNAME).a
	rm -f $(LIB_INSTALL)/$(LIBNAME).so
	rm -f $(LIB_INSTALL)/$(TARGET)
	rm -f $(LIBSDIR)/include/CNX_Hdcp.h
	rm -f ../libs/$(TARGET)

#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp)
INCS := $(INCLUDE)
depend dep:
	$(CC)  -M  $(CPPFLAGS)   $(INCS) $(SRCS) > .depend
