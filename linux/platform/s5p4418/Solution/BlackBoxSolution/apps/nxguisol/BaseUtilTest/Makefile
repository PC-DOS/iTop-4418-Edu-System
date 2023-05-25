ifndef TOPDIR
TOPDIR	:=	../..
endif
########################################################################
# Get Linux Build Enviornment:
include ../../../../../../build.env

######################################################################
# Build options
INCLUDE	+= -I./ -I../

LIBRARY	+= -lSDL -lSDL_ttf -lSDL_image
LIBRARY	+= -lstdc++ -lpthread

######################################################################
# Target
CPPOBJS	+= ../BaseComponent/NX_GuiUtils.o
CPPOBJS	+= NX_GuiUtilsTest.o

TARGET	:= nxguiutils
CPPFLAGS += -g

######################################################################
# Build
OBJS	:= $(COBJS) $(CPPOBJS)

all: $(TARGET)

$(TARGET): depend $(OBJS)
	$(CC) $(CPPFLAGS) $(OBJS) -o $@ $(LIBRARY)
	cp $(TARGET) ~/working/nfs/nxp4330/rootfs-tiny-sdl/root

clean:
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
	$(CC)  -M  $(CPPFLAGS)   $(INCS) $(SRCS) > .depend
