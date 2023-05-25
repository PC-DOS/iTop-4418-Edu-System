ifndef TOPDIR
TOPDIR	:=	../..
endif
########################################################################
# Get Linux Build Enviornment:
include ../../../../build.env

######################################################################
# Build options
INCLUDE += -I./ -I$(TOPDIR)/include
INCLUDE += -I$(LIBSDIR)/include
INCLUDE	+= -I./ -I./BaseComponent -I./nxdvrsol -I./nxplayersol

LIBRARY	+= -L$(TOPDIR)/lib
LIBRARY += -L$(LIBSDIR)/lib -L$(LIBSDIR)/lib/ratecontrol

# Dvr Library
LIBRARY += -lnxgraphictools -lEGL -lGLESv1_CM -lGLESv2 -lVR
LIBRARY += -lnxdvr -lnxnmeaparser -lnxaudio -lnxgpio -lnxadc

# Player Library
LIBRARY += -lnxmovplayer -lnxtypefind

# Common System Library
LIBRARY += -lnxvip -lnxdsp -lnxv4l2 -lnxvpu -lnxvidrc -lnxvmem

LIBRARY	+= -lSDL -lSDL_ttf -lSDL_image
LIBRARY += -lglib-2.0 -lgstreamer-0.10
LIBRARY	+= -lstdc++ -lpthread -lasound

######################################################################
# Target
# Dvr Component
CPPOBJS	+= \
	./nxdvrsol/NX_DvrMain.o				\
	./nxdvrsol/NX_Queue.o				\
	./nxdvrsol/NX_Semaphore.o 			\
	./nxdvrsol/NX_DvrCmdQueue.o			\
	./nxdvrsol/NX_DvrLedCtrl.o			\
	./nxdvrsol/NX_DvrFileManager.o 		\
	./nxdvrsol/NX_DvrFileQueue.o		\
	./nxdvrsol/NX_DvrGpsManager.o		\
	./nxdvrsol/NX_DvrGsensorManager.o	\
	./nxdvrsol/NX_DvrGsensor.o			\
	./nxdvrsol/NX_DvrPowerManager.o		\
	./nxdvrsol/NX_DvrConfig.o

# Player Component
CPPOBJS += \
	./nxplayersol/NX_PlayerMain.o		

# Base UI Component
CPPOBJS	+= \
	./BaseComponent/NX_DbgMsg.o			\
	./BaseComponent/NX_GuiUtils.o		\
	./BaseComponent/CNX_BaseWindow.o	\
	./BaseComponent/CNX_TextBox.o		\
	./BaseComponent/CNX_PushButton.o	\
	./BaseComponent/CNX_ProgressBar.o	\
	./BaseComponent/CNX_ToggleButton.o	\
	./BaseComponent/CNX_ListBox.o

# UI Component
CPPOBJS	+= \
	NX_MenuTop.o		\
	NX_MenuBlackBox.o	\
	NX_MenuFileList.o	\
	NX_MenuPlayer.o		\
	NX_MenuSetting.o	\
	NX_GuiMain.o		

TARGET	:= nxguisol
CPPFLAGS += -g

######################################################################
# Build
OBJS	:= $(COBJS) $(CPPOBJS)

all: $(TARGET)

$(TARGET): depend $(OBJS)
	$(CC) $(CPPFLAGS) $(OBJS) -o $@ $(LIBRARY)

clean:
	@find $(OBJTREE) -type f \
		\( -name '*.o'	-o -name $(TARGET) -o -name '*.map' -o -name '.depend' \) -print \
		| xargs rm -f

install:
	install -m 755 $(TARGET) $(TOPDIR)/bin
#	cp $(TARGET) ~/working/nfs/nxp4330/rootfs-gst-sdl/root

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
