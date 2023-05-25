.SUFFIXES : .c .o

INC =
LIBS =
CC = gcc
CFLAGS = -O2 $(INC)
LDFLAGS = -static



OBJS =	GEN_NANDBOOTEC.o \
		ProcessNSIH.o \
		nx_bingen.o
SRCS = 

TARGET = nx_bingen


.PHONY: clean

.c.o :
	$(CC) $(CFLAGS) -c $< -o $@

all : $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

dep :
	gccmakedep $(INC) $(SRCS)

clean :
	rm -rf $(OBJS) $(TARGET) core

new : 
	$(MAKE) clean 
	$(MAKE) 

