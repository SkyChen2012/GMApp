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
LIBNAME = libMXMenu.a
LIBNAMESO = libMXMenu.so

# List of objects to compile
OBJS = NetworkSetWnd.o LanguageSet.o MainMenu.o RoomCodeDigitSetWnd.o SysConMenu.o SysInfoWnd.o \
	SysOtherParaSetWnd.o SysPwdModifyWnd.o SysRestoreWnd.o TalkTimeSetWnd.o RingTimeSetWnd.o \
	VolumeSetWnd.o

endif

######################### Makefile.rules section #############################

include ../Makefile.rules
include $(TOP)/Makefile.rules 

######################## Tools targets section ###############################
