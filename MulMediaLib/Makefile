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
LIBNAME = libMXMulMediaLib.a
LIBNAMESO = libMXMulMediaLib.so

# List of objects to compile
OBJS = rtp.o udp.o vpu_capture.o vpu_codec.o vpu_display.o vpu_io.o \
	   vpu_lib.o vpu_voip_app.o 

endif
######################### Makefile.rules section #############################

include ../Makefile.rules
include $(TOP)/Makefile.rules 

######################## Tools targets section ###############################
