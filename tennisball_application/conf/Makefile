# File:		Makefile
# Author: 	Robinson Mittmann (bobmittmann@gmail.com)
# Copyright(C) 2010 CSP Automacao LTDA. All Rights Reserved.

PROJECT = libconf

CXXFILES = conf.cpp lookup.cpp

OFILES = $(CXXFILES:.cpp=.o)

INCPATH = ../include

OPTIONS = -g
CDEFS = DEBUG

SFLAGS  = -Wall
CFLAGS  = -O
LDFLAGS =

CXX     = arm-linux-gnueabihf-g++
LD      = arm-linux-gnueabihf-g++
AR      = ar
OBJCOPY = objcopy
OBJDUMP = objdump
STRIP = strip

LIB = $(PROJECT).a
LST = $(PROJECT).lst

CFLAGS := $(OPTIONS) $(CFLAGS) -Wall $(addprefix -D,$(CDEFS))

all: Makefile $(LIB) $(LST)

clean:
	@rm -fv $(OFILES) $(LIB) $(LST)

install: $(LIB)
	@if [ ! -d $(INSTALL_DIR) ]; then\
		mkdir $(INSTALL_DIR);\
	fi;\
	cp -f $(LIB) $(INSTALL_DIR)

$(LIB): $(OFILES)
	$(AR) $(ARFLAGS) $@ $?

$(LST): $(LIB)
	$(OBJDUMP) -w -D -t -S -r -z $< | sed '/^[0-9,a-f]\{8\} .[ ]*d[f]\?.*$$/d' > $@

%.o : %.cpp Makefile
	$(CXX) $(OPTIONS) $(CFLAGS) $(addprefix -I,$(INCPATH)) -o $@ -c $<

