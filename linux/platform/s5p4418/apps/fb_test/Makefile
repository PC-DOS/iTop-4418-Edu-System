
######################################################################
# Build options
LIBRARY	+=-lpthread
CFLAGS  += -static -W -O2
######################################################################
# Target
#COBJS  	:= cpu.o spi_test.o 
COBJS  	:= fb_test.o
CPPOBJS	:=
TARGET 	:= fb_test

CROSS_COMPILE = arm-cortex_a9-linux-gnueabi-
CC		= $(CROSS_COMPILE)gcc
#CC :=gcc 

######################################################################
# Build
OBJS	:= $(COBJS) $(CPPOBJS)
OBJS2	:= $(COBJS2) $(CPPOBJS2)

all: $(TARGET) $(TARGET2)

$(TARGET): depend $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LIBRARY) 

clean:
	@find $(OBJTREE) -type f \
		\( -name '*.o'	-o -name $(TARGET) -o -name $(TARGET2) -o -name '*.map' \) \
		-print | xargs rm -f

#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp)
SRCS += $(COBJS2:.o=.c) $(CPPOBJS2:.o=.cpp)
INCS := $(INCLUDE)
depend dep:
	$(CC)  -M  $(CFLAGS)   $(INCS) $(SRCS) > .depend
