########################################################################
# Get Linux Build Enviornment:
include ../../build.env

######################################################################
# Build options
INCLUDE += -I./
INCLUDE += -I../../library/include
LIBRARY	+= -L../../library/lib
LIBRARY += -lcec
LIBRARY	+= -lstdc++

######################################################################
# Target
COBJS	:= 
CPPOBJS	:=  cec_test.o
CECLOWOBJS := cec_low_test.o

CFLAGS	+= -g

TARGET := cec_test
CECLOWTEST := cec_low_test

######################################################################
# Build
OBJS	:= $(COBJS) $(CPPOBJS)

all: $(TARGET) $(CECLOWTEST)

$(TARGET):	depend $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LIBRARY)

$(CECLOWTEST): depend $(CECLOWOBJS)
	$(CC) $(CFLAGS) $(CECLOWOBJS) -o $@

clean:
	rm -f $(TARGET) *.o .depend

#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp) $(APPOBJS:.o=.c)
INCS := $(INCLUDE)
depend dep:
	$(CC) -M $(CFLAGS) $(INCS) $(SRCS) > .depend
