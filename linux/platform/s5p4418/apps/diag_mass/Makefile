########################################################################
# Set Install Directory
include ../../build.env

ifndef	INSTALL_DIR
INSTALL_DIR := ./__install
endif

########################################################################
# Module Definition & Version Management
DEVICE_NAME := S5P4418
MOD_BASE := diag_mass
MOD_NAME := s5p4418_diag_mass

BUILD_NUMBER_FILE = build_number_$(MOD_NAME)
BUILD_INFO_FILE = inc/build_info_$(MOD_NAME).h
MAJOR = 0
MINOR = 1


######################################################################
# Build options
INCLUDE += -I./ -I./inc

INCLUDE += -I$(KERNDIR)/include
INCLUDE += -I$(LIBSDIR)/src/libion
INCLUDE += -I$(LIBSDIR)/src/libnxv4l2

LIBRARY	+= -lstdc++ -ldl -lpthread -lasound -lSDL -lSDL_ttf
LIBRARY += -L$(LIB_INSTALL) -lnxvmem -lnxv4l2 -lnxdsp -lnxvip -lnxuevent

######################################################################
# Build

CPPOBJS	:=	src/utils.o			\
			src/DiagWindow.o	\
			src/CpuRamTest.o	\
			src/CameraTest.o	\
			src/HDMITest.o		\
			src/ButtonTest.o	\
			src/SensorTest.o	\
			src/AudioPlayTest.o	\
			src/AudioRecTest.o	\
			src/WiFiTest.o		\
			src/RTCTest.o		\
			src/main.o


OBJS	:= $(COBJS) $(CPPOBJS)


TARGET	:= nxdiag_mass

all: UPDATE_BUILD_NUMBER \
	$(TARGET)

$(TARGET): depend $(OBJS)
	$(CC) $(CPPFLAGS) $(OBJS) -o $@ $(LIBRARY)

clean:
	@find $(OBJTREE) -type f \
		\( -name '*.o'	-o -name $(TARGET) -o -name $(TARGET2) -o -name '*.map' \) \
		-print | xargs rm -f

install:
	mkdir -p $(INSTALL_DIR)
	mkdir -p $(INSTALL_DIR)/data
	mkdir -p $(INSTALL_DIR)/font
	mkdir -p $(INSTALL_DIR)/output
	install -m 777 $(TARGET) $(INSTALL_DIR)
	install -m 777 ./script/run.sh $(INSTALL_DIR)
	install -m 777 ./data/* $(INSTALL_DIR)/data
	install -m 777 ./font/* $(INSTALL_DIR)/font


UPDATE_BUILD_NUMBER :
	@echo $$(($$(cat $(BUILD_NUMBER_FILE).txt) + 1)) > $(BUILD_NUMBER_FILE).txt
	@echo
	@echo "Update Build Number: $(MOD_NAME) version $(MAJOR).$(MINOR).$$(cat $(BUILD_NUMBER_FILE).txt) (built by $$(whoami) at $$(date) )"
	@echo
	@echo "#define	DIAG_MODULE_NAME	$(MOD_NAME)	//	Module Name" > $(BUILD_INFO_FILE)
	@echo "#define	$(MOD_BASE)_DEVICE	\"$(DEVICE_NAME)\"	//	Device Name" >> $(BUILD_INFO_FILE)
	@echo "#define	$(MOD_BASE)_MAJOR	$(MAJOR)	//	Major Version Number" >> $(BUILD_INFO_FILE)
	@echo "#define	$(MOD_BASE)_MINOR	$(MINOR)	//	Minor Version Number" >> $(BUILD_INFO_FILE)
	@echo "#define	$(MOD_BASE)_BUILD	$$(cat $(BUILD_NUMBER_FILE).txt)	//	Build Number" >> $(BUILD_INFO_FILE)
	@echo "#define	$(MOD_BASE)_DATE	__DATE__" //	Build Date >> $(BUILD_INFO_FILE)
	@echo "#define	$(MOD_BASE)_TIME	__TIME__" //	Build Time >> $(BUILD_INFO_FILE)
	@echo "#define	$(MOD_BASE)_USER	\"$$(whoami)\""	//	Build User >> $(BUILD_INFO_FILE)

#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp)
INCS := $(INCLUDE)
depend dep:
	$(CC)  -M  $(CPPFLAGS)   $(INCS) $(SRCS) > .depend
