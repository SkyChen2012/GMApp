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
LIBNAME = libMXAccessProc.a
LIBNAMESO = libMXAccessProc.so

# List of objects to compile
OBJS = ASProcWnd.o ASPwdModWnd.o AccessProc.o ASRdPwdModWnd.o AsAlarmWnd.o \
		AccessLogReport.o ACAgentProc.o PwdUlk.o PwdUlkWnd.o PwdModifyWnd.o

endif

######################### Makefile.rules section #############################

include ../Makefile.rules
include $(TOP)/Makefile.rules 

######################## Tools targets section ###############################
