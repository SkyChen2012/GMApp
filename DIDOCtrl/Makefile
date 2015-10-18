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
LIBNAME = libMXDIDOCtrl.a
LIBNAMESO = libMXDIDOCtrl.so

# List of objects to compile
OBJS = modnet.o modbus.o crc32.o MoxDido.o DIDOClient.o 

endif

######################### Makefile.rules section #############################

include ../Makefile.rules
include $(TOP)/Makefile.rules 

######################## Tools targets section ###############################
