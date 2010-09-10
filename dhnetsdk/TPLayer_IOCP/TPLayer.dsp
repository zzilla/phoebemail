# Microsoft Developer Studio Project File - Name="TPLayer" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=TPLayer - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TPLayer.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TPLayer.mak" CFG="TPLayer - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TPLayer - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "TPLayer - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TPLayer - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../Bin/Release/"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# SUBTRACT CPP /Z<none> /Fr
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "TPLayer - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TPLayer___Win32_Debug"
# PROP BASE Intermediate_Dir "TPLayer___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../Bin/Debug/"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "TPLayer - Win32 Release"
# Name "TPLayer - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ITPObject.cpp
# End Source File
# Begin Source File

SOURCE=.\TPBroadcast.cpp
# End Source File
# Begin Source File

SOURCE=.\TPMulticastClient.cpp
# End Source File
# Begin Source File

SOURCE=.\TPTCPClient.cpp
# End Source File
# Begin Source File

SOURCE=.\TPTCPServer.cpp
# End Source File
# Begin Source File

SOURCE=.\TPUDPClient.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ITPListener.h
# End Source File
# Begin Source File

SOURCE=.\ITPObject.h
# End Source File
# Begin Source File

SOURCE=.\TPBroadcast.h
# End Source File
# Begin Source File

SOURCE=.\TPMulticastClient.h
# End Source File
# Begin Source File

SOURCE=.\TPTCPClient.h
# End Source File
# Begin Source File

SOURCE=.\TPTCPServer.h
# End Source File
# Begin Source File

SOURCE=.\TPTypedef.h
# End Source File
# Begin Source File

SOURCE=.\TPUDPClient.h
# End Source File
# Begin Source File

SOURCE=.\winsock2i.h
# End Source File
# End Group
# Begin Group "depend"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\include\atomiccount.cpp
# End Source File
# Begin Source File

SOURCE=.\include\atomiccount.h
# End Source File
# Begin Source File

SOURCE=.\include\AutoBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\include\AutoBuffer.h
# End Source File
# Begin Source File

SOURCE=.\include\Global.h
# End Source File
# Begin Source File

SOURCE=.\include\ReadWriteMutex.cpp
# End Source File
# Begin Source File

SOURCE=.\include\ReadWriteMutex.h
# End Source File
# Begin Source File

SOURCE=.\include\Referable.h
# End Source File
# End Group
# End Target
# End Project
