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
LIBNAME = libMXSoap.a
LIBNAMESO = libMXSoap.so

# List of objects to compile
OBJS = soapC.o soapClient.o soapClientLib.o soapServer.o soapServerLib.o soapcommon.o ns.o soapmsgserver.o soapmsgclient.o

endif

######################### Makefile.rules section #############################

include ../Makefile.rules
include $(TOP)/Makefile.rules 

######################## Tools targets section ###############################
