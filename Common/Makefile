##############################################################################
# Microwindows template Makefile
# Copyright (c) 2000 Martin Jolicoeur, Greg Haerr
##############################################################################

ifndef TOP
TOP = ../..
CONFIG = $(TOP)/config
endif

include $(CONFIG)

############################# targets section ################################

ifeq ($(MICROWIN), Y)

# If you want to create a library with the objects files, define the name here
LIBNAME = libMXCommon.a
LIBNAMESO = libMXCommon.so

# List of objects to compile
OBJS = MXMem.o MXList.o SystemInfoAPI.o IniFile.o watchdog.o CheckNetStatus.o \
	rtc.o RS485.o UnicodeString.o

endif

######################### Makefile.rules section #############################

include ../Makefile.rules
include $(TOP)/Makefile.rules

######################## Tools targets section ###############################

