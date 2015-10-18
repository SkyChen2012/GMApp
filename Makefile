##############################################################################
# Microwindows template Makefile
# Copyright (c) 2000 Martin Jolicoeur, Greg Haerr
##############################################################################

ifndef TOP
TOP = ..
CONFIG = $(TOP)/config
endif

include $(CONFIG)

######################## Additional Flags section ############################

#EthHVDir = $(TOP)/demos/mwin_ethhv
#CommonDir = $(EthHVDir)/Common
#DispatchDir = $(EthHVDir)/Dispatch

# Directories list for header files
INCLUDEDIRS += 
# Defines for preprocessor
DEFINES +=

# Compilation flags for C files OTHER than include directories
CFLAGS +=
# Preprocessor flags OTHER than defines
CPPFLAGS +=
# Linking flags
#LDFLAGS +=
LDFLAGS += --static
############################# targets section ################################

# If you want to create a library with the objects files, define the name here
LIBNAME =

# List of objects to compile
OBJS =

#ifeq ($(ARCH), ECOS)
#dirs = nanowm nanox nxkbd nxscribble
#endif

all: 
	$(MAKE) -C $(DIDOCtrlDir)
	$(MAKE) -C $(CommonDir)
	$(MAKE) -C $(AccessCommonDir)
	$(MAKE) -C $(DispatchDir)
	$(MAKE) -C $(PioApiDir)
	$(MAKE) -C $(TalkingDir)
	$(MAKE) -C $(EthDir)
	$(MAKE) -C $(MulMediaLibDir)
	$(MAKE) -C $(MultiMediaDir)
	$(MAKE) -C $(CardProcDir)
	$(MAKE) -C $(AccessProcDir)
	$(MAKE) -C $(ParaSettingDir)
	$(MAKE) -C $(MenuDir)
	$(MAKE) -C $(JpegLibDir)
	$(MAKE) -C $(TelnetdDir)
	$(MAKE) -C $(AccessSettingDir)
	$(MAKE) -C $(PatrolDir)
	$(MAKE) -C $(LiftControlDir)
	$(MAKE) -C $(SecurityAlarmDir)
	$(MAKE) -C $(GSoapDir)
	$(MAKE) -C $(SoapDir)
	$(MAKE) -C $(MainDir)
	

	cp -af $(MainDir)/EthGM ./
	
	tar -czvf a.tgz EthGM
	#cp -af a.tgz /tftpboot/
	#cp -af $(MainDir)/EthGM /tftpboot/mox/

######################### Makefile.rules section #############################

include ./Makefile.rules 
include $(TOP)/Makefile.rules 

######################## Tools targets section ###############################

