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
LIBNAME = libMXLiftControl.a
LIBNAMESO = libMXLiftControl.so

# List of objects to compile
OBJS = BacpSerial.o BacpSerialApp.o EGMLCAgent.o  EGMLiftControl.o LiftCtrlConversion.o \
	LCAPro_mox_V1.o LCAPro_mitsubishi.o InCarCardProc.o LCAPro_mox_V2.o LiftCtrlByDOModule.o

endif

######################### Makefile.rules section #############################

include ../Makefile.rules
include $(TOP)/Makefile.rules 

######################## Tools targets section ###############################
