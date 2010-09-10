# Microsoft Developer Studio Project File - Name="netsdk" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=NETSDK - WIN32 RELEASE
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "netsdk.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "netsdk.mak" CFG="NETSDK - WIN32 RELEASE"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "netsdk - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "netsdk - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "netsdk - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../Bin/Release/"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NETSDK_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NETSDK_EXPORTS" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /pdb:none /machine:I386 /def:".\netsdk.def"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "netsdk - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../Bin/Debug/"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NETSDK_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NETSDK_EXPORTS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /def:".\netsdk.def" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "netsdk - Win32 Release"
# Name "netsdk - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AlarmDeal.cpp
# End Source File
# Begin Source File

SOURCE=.\Alaw_encoder.c
# End Source File
# Begin Source File

SOURCE=.\AutoRegister.cpp
# End Source File
# Begin Source File

SOURCE=.\DecoderDevice.cpp
# End Source File
# Begin Source File

SOURCE=.\DevConfig.cpp
# End Source File
# Begin Source File

SOURCE=.\DevConfigEx.cpp
# End Source File
# Begin Source File

SOURCE=.\DevControl.cpp
# End Source File
# Begin Source File

SOURCE=.\DevNewConfig.cpp
# End Source File
# Begin Source File

SOURCE=.\GPSSubcrible.cpp
# End Source File
# Begin Source File

SOURCE=.\Manager.cpp
# End Source File
# Begin Source File

SOURCE=..\dvr\mutex.cpp
# End Source File
# Begin Source File

SOURCE=.\netsdk.cpp

!IF  "$(CFG)" == "netsdk - Win32 Release"

!ELSEIF  "$(CFG)" == "netsdk - Win32 Debug"

# ADD CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\netsdk.def
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\netsdk.rc
# End Source File
# Begin Source File

SOURCE=..\dvr\osIndependent.cpp
# End Source File
# Begin Source File

SOURCE=..\dvr\ParseString.cpp
# End Source File
# Begin Source File

SOURCE=..\dvr\ReadWriteMutex.cpp
# End Source File
# Begin Source File

SOURCE=.\RealPlay.cpp
# End Source File
# Begin Source File

SOURCE=.\RenderManager.cpp
# End Source File
# Begin Source File

SOURCE=.\SearchRecordAndPlayBack.cpp
# End Source File
# Begin Source File

SOURCE=.\ServerSet.cpp
# End Source File
# Begin Source File

SOURCE=.\SnapPicture.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\Talk.cpp
# End Source File
# Begin Source File

SOURCE=.\Timer.cpp
# End Source File
# Begin Source File

SOURCE=.\TransBurnFile.cpp
# End Source File
# Begin Source File

SOURCE=.\Utils.cpp
# End Source File
# Begin Source File

SOURCE=.\Utils_StrParser.cpp
# End Source File
# Begin Source File

SOURCE=.\VideoRender.cpp

!IF  "$(CFG)" == "netsdk - Win32 Release"

!ELSEIF  "$(CFG)" == "netsdk - Win32 Debug"

# ADD CPP /YX

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AlarmDeal.h
# End Source File
# Begin Source File

SOURCE=.\assistant.h
# End Source File
# Begin Source File

SOURCE=.\AutoRegister.h
# End Source File
# Begin Source File

SOURCE=.\DecoderDevice.h
# End Source File
# Begin Source File

SOURCE=.\DevConfig.h
# End Source File
# Begin Source File

SOURCE=.\DevConfigEx.h
# End Source File
# Begin Source File

SOURCE=.\DevControl.h
# End Source File
# Begin Source File

SOURCE=.\DevNewConfig.h
# End Source File
# Begin Source File

SOURCE=.\Eng_assistant.h
# End Source File
# Begin Source File

SOURCE=.\Eng_netsdk.h
# End Source File
# Begin Source File

SOURCE=.\GPSSubcrible.h
# End Source File
# Begin Source File

SOURCE=.\Manager.h
# End Source File
# Begin Source File

SOURCE=..\dvr\mutex.h
# End Source File
# Begin Source File

SOURCE=.\netsdk.h
# End Source File
# Begin Source File

SOURCE=.\netsdktypes.h
# End Source File
# Begin Source File

SOURCE=..\dvr\ReadWriteMutex.h
# End Source File
# Begin Source File

SOURCE=.\RealPlay.h
# End Source File
# Begin Source File

SOURCE=.\RenderManager.h
# End Source File
# Begin Source File

SOURCE=.\SearchRecordAndPlayBack.h
# End Source File
# Begin Source File

SOURCE=.\ServerSet.h
# End Source File
# Begin Source File

SOURCE=.\SnapPicture.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\Talk.h
# End Source File
# Begin Source File

SOURCE=.\Timer.h
# End Source File
# Begin Source File

SOURCE=.\TransBurnFile.h
# End Source File
# Begin Source File

SOURCE=.\Utils.h
# End Source File
# Begin Source File

SOURCE=.\Utils_StrParser.h
# End Source File
# Begin Source File

SOURCE=.\VideoRender.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "NetPlayBack"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\NetPlayBack\NetPlayBack.cpp
# End Source File
# Begin Source File

SOURCE=.\NetPlayBack\NetPlayBack.h
# End Source File
# Begin Source File

SOURCE=.\NetPlayBack\NetPlayBackBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\NetPlayBack\NetPlayBackBuffer.h
# End Source File
# End Group
# Begin Group "Depends"

# PROP Default_Filter "lib"
# Begin Source File

SOURCE=.\depend\amrnb.lib
# End Source File
# Begin Source File

SOURCE=.\depend\playsdk.lib
# End Source File
# End Group
# End Target
# End Project
