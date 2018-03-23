shell = /bin/sh
PWD_DIR=$(shell pwd)

LIB_DIR=$(PWD_DIR)/../../lib/
OBJECT_DIR=$(PWD_DIR)/../../build/
RUDP_DIR=$(PWD_DIR)/../../rudp/

RM=rm -rf
CCXX=g++
CC=gcc
AR = ar rc
SRC_OBJECT=$(wildcard $(RUDP_DIR)*.c) $(wildcard $(RUDP_DIR)nd_core/*.c)

SRCS=$(notdir $(SRC_OBJECT))

H_OBJECT=$(wildcard $(RUDP_DIR)*.h) $(wildcard $(RUDP_DIR)nd_core/*.h)

OBJECT=$(patsubst %.c,%.o,$(SRCS))

DY_SRC=$(wildcard *.cpp)

DY_SRCS=$(notdir $(DY_SRC))
OBJECTS=$(patsubst %.cpp,%.o,$(DY_SRCS))

LIB_OBJECT=librudp.a
#LIB_OBJECT_SO=librudp.so
BIN_OBJECT=recv_rudp

.PHONY:all
all:$(LIB_OBJECT) $(BIN_OBJECT)

$(LIB_OBJECT):$(OBJECT)
	$(AR) -o $@ $(PWD_DIR)/*.o
	mv $(LIB_OBJECT) $(LIB_DIR)
	mv *.o $(OBJECT_DIR)

$(OBJECT):$(SRC_OBJECT)
	$(CC) -c $(SRC_OBJECT)

$(BIN_OBJECT):$(OBJECTS)
	$(CCXX) -o $(BIN_OBJECT) $(OBJECTS) $(LIB_DIR)$(LIB_OBJECT) -lpthread
	#$(CC) -o $(BIN_OBJECT) $(OBJECTS) ../lib/$(LIB_OBJECT_SO)
	#$(CC) $(OBJECTS) -L$(LIB_DIR) -Itest -o $(BIN_OBJECT) 

$(OBJECTS):$(DY_SRC)
	$(CCXX) -c $(DY_SRC)



clean:
	$(RM) $(LIB_DIR)$(LIB_OBJECT) $(OBJECT_DIR)*.o $(BIN_OBJECT) *.o
