# Microsoft Developer Studio Project File - Name="MulMediaLib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Generic Project" 0x010a

CFG=MulMediaLib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MulMediaLib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MulMediaLib.mak" CFG="MulMediaLib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MulMediaLib - Win32 Release" (based on "Win32 (x86) Generic Project")
!MESSAGE "MulMediaLib - Win32 Debug" (based on "Win32 (x86) Generic Project")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
MTL=midl.exe

!IF  "$(CFG)" == "MulMediaLib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "MulMediaLib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "MulMediaLib - Win32 Release"
# Name "MulMediaLib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\rtp.c
# End Source File
# Begin Source File

SOURCE=.\udp.c
# End Source File
# Begin Source File

SOURCE=.\vpu_capture.c
# End Source File
# Begin Source File

SOURCE=.\vpu_codec.c
# End Source File
# Begin Source File

SOURCE=.\vpu_display.c
# End Source File
# Begin Source File

SOURCE=.\vpu_io.c
# End Source File
# Begin Source File

SOURCE=.\vpu_lib.c
# End Source File
# Begin Source File

SOURCE=.\vpu_voip_app.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\rtp.h
# End Source File
# Begin Source File

SOURCE=.\udp.h
# End Source File
# Begin Source File

SOURCE=.\vpu_capture.h
# End Source File
# Begin Source File

SOURCE=.\vpu_codetable.h
# End Source File
# Begin Source File

SOURCE=.\vpu_display.h
# End Source File
# Begin Source File

SOURCE=.\vpu_io.h
# End Source File
# Begin Source File

SOURCE=.\vpu_lib.h
# End Source File
# Begin Source File

SOURCE=.\vpu_voip_app.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Makefile
# End Source File
# End Target
# End Project
