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
LIBNAME = libMXAccessSetting.a
LIBNAMESO = libMXAccessSetting.so

# List of objects to compile
OBJS = AccessSystemWnd.o AddCardWnd.o ASSetWnd.o DelCardModeWnd.o DelRdModeWnd.o \
	DelLocalNumModeWnd.o DelModeSelWnd.o GataParaSetWnd.o GateOvertimeSetWnd.o \
	GateOpenModeSetWnd.o GatePulseWidthSetWnd.o IvdCardSwipeSetWnd.o \
	ManOpenSetWnd.o CardMngWnd.o InfraredSetWnd.o

endif

######################### Makefile.rules section #############################

include ../Makefile.rules
include $(TOP)/Makefile.rules 

######################## Tools targets section ###############################
