ifndef TOPDIR
TOPDIR	:= ../../../..
endif

########################################################################
# Get Linux Build Enviornment:
include $(TOPDIR)/build.env

######################################################################
# Build options
INCLUDE += -I./ -I../../inc

LIBRARY	+= -lstdc++ -ldl -lpthread -lSDL -lSDL_ttf
LIBRARY	+= -L../../libs -lnxdiagcommon

######################################################################
# Target
COBJS  	:= 
# add sdal test module
CPPOBJS	:= 
CPPOBJS	+= top_window.o
CPPOBJS	+= main.o

TARGET 	:= nxdiagnostics 
CPPFLAGS += -g

######################################################################
# Build
OBJS	:= $(COBJS) $(CPPOBJS)

#UPDATE_BUILD_NUMBER :
#	@echo $$(($$(cat $(BUILD_NUMBER_FILE).txt) + 1)) > $(BUILD_NUMBER_FILE).txt
#	@echo
#	@echo "Update Build Number: $(APP_NAME) version $(MAJOR).$(MINOR).$$(cat $(BUILD_NUMBER_FILE).txt) (built by $$(whoami) at $$(date) )"
#	@echo
#	@echo "const char* __$(APP_NAME)_VERSION_STRING__ = \"$(MOD_NAME) version $(MAJOR).$(MINOR).$$(cat $(BUILD_NUMBER_FILE).txt) (built by $$(whoami) at \" __DATE__ \",\" __TIME__ \")\";" > $(BUILD_NUMBER_FILE).c

all: $(TARGET)

$(TARGET): depend $(OBJS)
	$(CC) $(CPPFLAGS) $(OBJS) -o $@ $(LIBRARY)
	install -m 755 $(TARGET) ../../bin

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
