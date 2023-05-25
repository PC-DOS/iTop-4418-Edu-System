ifndef TOPDIR
TOPDIR	:=	../..
endif
########################################################################
# Get Linux Build Enviornment:
include ../../../../build.env

######################################################################
# Build options
JOBS := 4

# Prevent: "warning: creating a DT_TEXTREL in object"
CFLAGS	 += -fpic -DARM_11
CPPFLAGS += -fpic -DARM_11
# Debug options
ifdef DEBUG
ifeq ($(DEBUG), y)
CFLAGS	 += -DDEBUG -g
CPPFLAGS += -DDEBUG -g
endif
endif
INCLUDE		+= -I./
INCLUDE		+= -I$(LIBSDIR)/include
INCLUDE		+= -I$(TOPDIR)/include

INCLUDE 	+= -I$(TOPDIR)/include/live555/liveMedia
INCLUDE 	+= -I$(TOPDIR)/include/live555/groupsock
INCLUDE 	+= -I$(TOPDIR)/include/live555/BasicUsageEnvironment
INCLUDE 	+= -I$(TOPDIR)/include/live555/UsageEnvironment

######################################################################
# Target
# Test Component

# Common modules
CPPOBJS		+= NX_FilterSysApi.o NX_DbgMsg.o

# Video modules
CPPOBJS		+= CNX_VIPFilter.o
CPPOBJS		+= CNX_VRFilter.o
CPPOBJS		+= CNX_H264Encoder.o
# Audio modules
CPPOBJS		+= CNX_AudCaptureFilter.o
CPPOBJS		+= CNX_AacEncoder.o
CPPOBJS		+= CNX_Mp3Encoder.o
# Event Buffering & Interleaver modules
CPPOBJS		+= CNX_InterleaverFilter.o
CPPOBJS		+= CNX_BufferingFilter.o
# Muxer & FilterWriter modules
CPPOBJS		+= CNX_Mp4MuxerFilter.o
CPPOBJS		+= CNX_TsMuxerFilter.o
CPPOBJS		+= CNX_FileWriter.o
CPPOBJS		+= CNX_SimpleFileWriter.o
# User data & Textoverlay module
CPPOBJS		+= CNX_UserDataFilter.o
CPPOBJS 	+= CNX_TextOverlayFilter.o
CPPOBJS		+= CNX_ImageEffectFilter.o
# Etc Module (Network, MotionDetect, ...)
CPPOBJS		+= CNX_DynamicRTSPServer.o
CPPOBJS		+= CNX_RTPFilter.o
CPPOBJS		+= CNX_HLSFilter.o
CPPOBJS		+= CNX_MotionDetectFilter.o
#Transcoding module
CPPOBJS		+= codec_info.o media_reader.o
CPPOBJS		+= CNX_VideoDecoder.o

LIBNAME 	:= libnxfilters
TARGET		:= $(LIBNAME).a
######################################################################
# Build
OBJS	:= $(COBJS) $(CPPOBJS)

all: $(TARGET)

$(TARGET): depend $(OBJS)
	$(AR) $(ARFLAGS) $(TARGET) $(OBJS)

clean:
	@find $(OBJTREE) -type f \
		\( -name '*.o'	-o -name $(TARGET) -o -name '*.a' -o -name '*.so.*' -o \
		-name '*.bin' -o -name '*.map' -o -name 'core' -o -name '*~' -o -name '.depend' \) \
		-print | xargs rm -f

install:
	@echo "No Operation."

distclean:
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
