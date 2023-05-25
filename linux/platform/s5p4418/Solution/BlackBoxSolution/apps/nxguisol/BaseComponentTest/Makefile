ifndef TOPDIR
TOPDIR	:=	../..
endif
########################################################################
# Get Linux Build Enviornment:
include ../../../../../../build.env

######################################################################
# Build options
INCLUDE	+= -I./ -I../BaseComponent

LIBRARY	+= -lSDL -lSDL_ttf -lSDL_image
LIBRARY	+= -lstdc++ -lpthread

######################################################################
# Target
CPPOBJS	+= \
	../BaseComponent/NX_DbgMsg.cpp			\
	../BaseComponent/NX_GuiUtils.cpp		\
	../BaseComponent/CNX_BaseWindow.cpp		\
	../BaseComponent/CNX_TextBox.cpp		\
	../BaseComponent/CNX_PushButton.cpp		\
	../BaseComponent/CNX_ProgressBar.cpp	\
	../BaseComponent/CNX_ToggleButton.cpp	\
	../BaseComponent/CNX_ListBox.cpp
CPPOBJS	+= NX_GuiComponentTest.o

TARGET	:= nxguitest
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

install:
	install -m 755 $(TARGET) $(TOPDIR)/bin

distclean:
	rm -f $(TOPDIR)/bin/$(TARGET)
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
