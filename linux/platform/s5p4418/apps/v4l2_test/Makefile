########################################################################
# Get Linux Build Enviornment:
include ../../build.env

ifndef KERNDIR
KERNDIR := /home/ray/devel/android/jb-mr1.1/kernel
endif

######################################################################
# Build options
INCLUDE += -I./
INCLUDE += -I$(KERNDIR)/include
INCLUDE += -I$(LIBSDIR)/src/libion
INCLUDE += -I$(LIBSDIR)/src/libnxv4l2
INCLUDE += -I../../library/include
LIBRARY	+= -L../../library/lib

LIBRARY	+= -L$(LIBSDIR)/src/libion
LIBRARY	+= -L$(LIBSDIR)/src/libnxv4l2

LIBRARY += -lion -lv4l2-nexell
LIBRARY	+= -lm -lstdc++

######################################################################
# Target
CSI_OBJS	:= test-csi.o

DECI_OBJS	:= test-decimator.o

DECI_CROP_OBJS := test-decimator-source-crop.o

DECI_CROP_SCALE_OBJS := test-decimator-source-crop-scaledown.o

HDMI_OBJS	:= test-hdmi.o

RESC_OBJS	:= 
#RESC_OBJS	:= test-resc.o

CAM_OBJS	:= test-cam.o

CAM_DECI_CLIPPER_OBJS := test-cam-deci-clipper.o

CAM_INTERLACE_OBJS := test-cam-interlace-hdmi.o

CAM_VIDEO_SOURCE_CROP_OBJS := test-cam-video-out-source-crop.o

CAM_CLIPPER_CROP_OBJS := test-cam-clipper-crop.o

COBJS	:= 

CPPOBJS	:= $(CSI_OBJS) $(DECI_OBJS) $(DECI_CROP_OBJS) $(DECI_CROP_SCALE_OBJS) $(HDMI_OBJS) $(RESC_OBJS) $(CAM_OBJS) $(CAM_INTERLACE_OBJS) $(CAM_VIDEO_SOURCE_CROP_OBJS) $(CAM_DECI_CLIPPER_OBJS) $(CAM_CLIPPER_CROP_OBJS)

CFLAGS	+= -g

CAMAPP  := camera_test
DECIAPP := decimator_test
DECICROPAPP := decimator-source-crop-test
DECICROPSCALEAPP := decimator-source-crop-scale-test
CSIAPP  := csi_test
HDMIAPP := hdmi_test
RESCAPP := resc_test
CAMINTERLACEAPP := camera-interlace-hdmi-test
CAMVIDEOOUTSOURCECROPAPP := camera-video-out-source-crop-test
CAMDECICLIPPERAPP := camera-deci-clipper-test
CAMCLIPPERCROPAPP := camera-clipper-crop-test

######################################################################
# Build
OBJS	:= $(COBJS) $(CPPOBJS)

all: $(CSIAPP) $(DECIAPP) $(DECICROPAPP) $(DECICROPSCALEAPP) $(HDMIAPP) $(CAMAPP) $(CAMINTERLACEAPP) $(CAMVIDEOOUTSOURCECROPAPP) $(CAMDECICLIPPERAPP) $(CAMCLIPPERCROPAPP)

$(DECIAPP):	depend $(OBJS)
	$(CC) $(CPPFLAGS) $(DECI_OBJS) -o $@ $(LIBRARY)

$(DECICROPAPP):	depend $(OBJS)
	$(CC) $(CPPFLAGS) $(DECI_CROP_OBJS) -o $@ $(LIBRARY)

$(DECICROPSCALEAPP):	depend $(OBJS)
	$(CC) $(CPPFLAGS) $(DECI_CROP_SCALE_OBJS) -o $@ $(LIBRARY)

$(HDMIAPP):	depend $(OBJS)
	$(CC) $(CPPFLAGS) $(HDMI_OBJS) -o $@ $(LIBRARY)

$(CSIAPP):	depend $(OBJS)
	$(CC) $(CPPFLAGS) $(CSI_OBJS) -o $@ $(LIBRARY)

$(RESCAPP):	depend $(OBJS)
	$(CC) $(CPPFLAGS) $(RESC_OBJS) -o $@ $(LIBRARY)

$(CAMAPP):	depend $(OBJS)
	$(CC) $(CPPFLAGS) $(CAM_OBJS) -o $@ $(LIBRARY)

$(CAMINTERLACEAPP):	depend $(OBJS)
	$(CC) $(CPPFLAGS) $(CAM_INTERLACE_OBJS) -o $@ $(LIBRARY)

$(CAMVIDEOOUTSOURCECROPAPP):	depend $(OBJS)
	$(CC) $(CPPFLAGS) $(CAM_VIDEO_SOURCE_CROP_OBJS) -o $@ $(LIBRARY)

$(CAMDECICLIPPERAPP):	depend $(OBJS)
	$(CC) $(CPPFLAGS) $(CAM_DECI_CLIPPER_OBJS) -o $@ $(LIBRARY)

$(CAMCLIPPERCROPAPP):	depend $(OBJS)
	$(CC) $(CPPFLAGS) $(CAM_CLIPPER_CROP_OBJS) -o $@ $(LIBRARY)

clean:
	rm -f $(DECIAPP) $(HDMIAPP) $(CSIAPP) $(RESCAPP) *.o .depend

#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp) $(APPOBJS:.o=.c)
INCS := $(INCLUDE)
depend dep:
	$(CC) -M $(CPPFLAGS) $(INCS) $(SRCS) > .depend
