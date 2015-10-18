##############################################################################
# Microwindows template Makefile
# Copyright (c) 2000 Martin Jolicoeur, Greg Haerr
##############################################################################

include $(CONFIG)

######################## Additional Flags section ############################

# Directories list for header files
INCLUDEDIRS += 
# Defines for preprocessor
DEFINES +=

# Compilation flags for C files OTHER than include directories
CFLAGS += -I$(DispatchDir) -I$(CommonDir)
# Preprocessor flags OTHER than defines
CPPFLAGS +=
# Linking flags
LDFLAGS += --static

############################# targets section ################################

MXLIBS = 	$(TOP)/lib/libmwinlib.a \
	$(TOP)/lib/libMXDispatch.a \
	$(TOP)/lib/libMXCommon.a \
	$(TOP)/lib/libMXEth.a \
	$(TOP)/lib/libMXTalking.a \
	$(TOP)/lib/libMXMultiMedia.a \
	$(TOP)/lib/libMXAccessCommon.a \
	$(TOP)/lib/libMXParaSetting.a \
	$(TOP)/lib/libMXMulMediaLib.a \
	$(TOP)/lib/libMXPioApi.a \
	$(TOP)/lib/libMXMenu.a \
	$(TOP)/lib/libMXJpegLib.a \
	$(TOP)/lib/libMXTelnet.a \
	$(TOP)/lib/libMXAccessSetting.a \
	$(TOP)/lib/libMXPatrol.a \
	$(TOP)/lib/libMXAccessProc.a \
	$(TOP)/lib/libMXCardProc.a \
	$(TOP)/lib/libMXLiftControl.a \
	$(TOP)/lib/libMXSecurityAlarm.a \
	$(TOP)/lib/libMXDIDOCtrl.a \
	$(TOP)/lib/libMXSoap.a \
	$(TOP)/lib/libMXGSoap.a \
	
CCMXLIBS = -lMXDispatch \
	-lMXCommon \
	-lpthread \
	-lMXEth \
	-lMXTalking \
	-lMXMultiMedia \
	-lMXAccessCommon \
	-lMXAccessProc \
	-lMXCardProc \
	-lMXGSoap \
	-lMXSoap \
	-lMXParaSetting \
	-lMXMulMediaLib \
	-lMXSecurityAlarm \
	-lMXPioApi \
	-lMXMenu \
	-lMXJpegLib \
	-lMXTelnet \
	-lMXAccessSetting \
	-lMXPatrol \
	-lMXLiftControl \
	-lmwinlib \


ifeq ($(MICROWIN), Y)

# If you want to create a library with the objects files, define the name here
LIBNAME =

# List of objects to compile
OBJS =	EthGMMain.o HelpInfoWnd.o SysLogProc.o

MINEIMAGES = mineflag.o mineface.o minefacelost.o minebomb.o minedone.o\
	minehitfalse.o

TARGET = EthGM

all: default $(TARGET)

endif

######################### Makefile.rules section #############################

include ../Makefile.rules
include $(TOP)/Makefile.rules

######################## Tools targets section ###############################

$(TARGET): $(OBJS) $(MWINLIBS) $(MXLIBS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ $(CCMWINLIBS) $(CCMXLIBS)

