shell = /bin/sh

PWD_DIR=$(shell pwd)

LIB_DIR=$(PWD_DIR)/lib/
OBJECT_DIR=$(PWD_DIR)/build/
RUDP_DIR=$(PWD_DIR)/rudp/

RM=rm -fr

#****************************************************************************
#CFLAGS=-Wall -O3
CC = gcc
AR = ar rc
SHARED=-shared -o
FPIC=-fpic -c

SRC_OBJECT=$(wildcard ./rudp/*.c) $(wildcard ./rudp/nd_core/*.c)

SRCS=$(notdir $(SRC_OBJECT))

H_OBJECT=$(wildcard ./rudp/*.h) $(wildcard ./rudp/nd_core/*.h)

OBJECT=$(patsubst %.c,%.o,$(SRCS))


LIB_OBJECT = librudp.a
LIB_OBJECT_SO=librudp.so
target=
#test:
#	echo $(SRC_OBJECT)
#	echo $(SRCS)
#	echo $(H_OBJECT)
#	echo $(OBJECT)

#****************************************************************************
#.PHONY:all
#all:$(LIB_OBJECT_SO)

ifeq ($(target), so)
$(LIB_OBJECT_SO):$(OBJECT)
	$(CC) $(SHARED) $@ $(PWD_DIR)/*.o
	mv $(LIB_OBJECT_SO) $(LIB_DIR)
	mv *.o $(OBJECT_DIR)

$(OBJECT):$(SRC_OBJECT)
	$(CC) $(FPIC) $(SRC_OBJECT)
else
$(LIB_OBJECT):$(OBJECT)
	$(AR) -o $@ $(PWD_DIR)/*.o
	mv $(LIB_OBJECT) $(LIB_DIR)
	mv *.o $(OBJECT_DIR)

$(OBJECT):$(SRC_OBJECT)
	$(CC) -c $(SRC_OBJECT)

endif
clean:
	$(RM) $(OBJECT_DIR)*
	$(RM) $(LIB_DIR)*
	$(RM) *.o





