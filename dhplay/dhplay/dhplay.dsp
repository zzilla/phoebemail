# Microsoft Developer Studio Project File - Name="dhplay" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=dhplay - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "dhplay.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dhplay.mak" CFG="dhplay - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dhplay - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "dhplay - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "dhplay - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DHPLAY_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Zi /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DHPLAY_EXPORTS" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 vfw32.lib winmm.lib dxguid.lib ddraw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /pdb:none /machine:I386 /out:"../Bin/Release/playsdk.dll"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "dhplay - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DHPLAY_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MD /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DHPLAY_EXPORTS" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 vfw32.lib winmm.lib dxguid.lib ddraw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /def:".\dhplay.def" /out:"../Bin/Debug\playsdk.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "dhplay - Win32 Release"
# Name "dhplay - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "YUV2PICS"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\YUV2PICS\JpegEncoder.c
# End Source File
# Begin Source File

SOURCE=.\YUV2PICS\JpegEncoder.h
# End Source File
# Begin Source File

SOURCE=.\YUV2PICS\JpegInfo.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\AccurateTimer.cpp
# End Source File
# Begin Source File

SOURCE=.\callback.cpp
# End Source File
# Begin Source File

SOURCE=.\data.cpp
# End Source File
# Begin Source File

SOURCE=.\dhplay.cpp
# End Source File
# Begin Source File

SOURCE=.\dhplay.def

!IF  "$(CFG)" == "dhplay - Win32 Release"

!ELSEIF  "$(CFG)" == "dhplay - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\DHSTDStreamFileOpr.cpp
# End Source File
# Begin Source File

SOURCE=.\display.cpp
# End Source File
# Begin Source File

SOURCE=.\HI_PLAY_AudioIN.cpp
# End Source File
# Begin Source File

SOURCE=.\NewStreamFileOpr.cpp
# End Source File
# Begin Source File

SOURCE=.\oldStreamfileOpr.cpp
# End Source File
# Begin Source File

SOURCE=.\play.cpp
# End Source File
# Begin Source File

SOURCE=.\playmanage.cpp
# End Source File
# Begin Source File

SOURCE=.\ShStreamFileOpr.cpp
# End Source File
# Begin Source File

SOURCE=.\StandardStreamFirOpr.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# End Source File
# Begin Source File

SOURCE=.\streampaser.rc
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AccurateTimer.h
# End Source File
# Begin Source File

SOURCE=.\callback.h
# End Source File
# Begin Source File

SOURCE=.\data.h
# End Source File
# Begin Source File

SOURCE=.\dhplay.h
# End Source File
# Begin Source File

SOURCE=.\dhplayEx.h
# End Source File
# Begin Source File

SOURCE=.\DHSTDStreamFileOpr.h
# End Source File
# Begin Source File

SOURCE=.\display.h
# End Source File
# Begin Source File

SOURCE=.\fileoprinterface.h
# End Source File
# Begin Source File

SOURCE=.\HI_PLAY_AudioIN.h
# End Source File
# Begin Source File

SOURCE=C:\mssdk\include\multimon.h
# End Source File
# Begin Source File

SOURCE=.\NewStreamFileOpr.h
# End Source File
# Begin Source File

SOURCE=.\oldStreamfileOpr.h
# End Source File
# Begin Source File

SOURCE=.\play.h
# End Source File
# Begin Source File

SOURCE=.\playmanage.h
# End Source File
# Begin Source File

SOURCE=.\ShStreamFileOpr.h
# End Source File
# Begin Source File

SOURCE=.\StandardStreamFirOpr.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\utils.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "decode"

# PROP Default_Filter ""
# Begin Group "DhStreamParser"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\decode\DhStreamParser\asfstream.cpp
# End Source File
# Begin Source File

SOURCE=.\decode\DhStreamParser\asfstream.h
# End Source File
# Begin Source File

SOURCE=.\decode\DhStreamParser\dhstdstream.cpp
# End Source File
# Begin Source File

SOURCE=.\decode\DhStreamParser\dhstdstream.h
# End Source File
# Begin Source File

SOURCE=.\decode\DhStreamParser\DhStreamParser.cpp
# End Source File
# Begin Source File

SOURCE=.\decode\DhStreamParser\DhStreamParser.h
# End Source File
# Begin Source File

SOURCE=.\decode\DhStreamParser\FrameList.h
# End Source File
# Begin Source File

SOURCE=.\decode\DhStreamParser\newstream.cpp
# End Source File
# Begin Source File

SOURCE=.\decode\DhStreamParser\newstream.h
# End Source File
# Begin Source File

SOURCE=.\decode\DhStreamParser\oldstream.cpp
# End Source File
# Begin Source File

SOURCE=.\decode\DhStreamParser\oldstream.h
# End Source File
# Begin Source File

SOURCE=.\decode\DhStreamParser\psstream.cpp
# End Source File
# Begin Source File

SOURCE=.\decode\DhStreamParser\psstream.h
# End Source File
# Begin Source File

SOURCE=.\decode\DhStreamParser\rwstream.cpp
# End Source File
# Begin Source File

SOURCE=.\decode\DhStreamParser\rwstream.h
# End Source File
# Begin Source File

SOURCE=.\decode\DhStreamParser\shstream.cpp
# End Source File
# Begin Source File

SOURCE=.\decode\DhStreamParser\shstream.h
# End Source File
# Begin Source File

SOURCE=.\decode\DhStreamParser\StdMp4Parse.cpp
# End Source File
# Begin Source File

SOURCE=.\decode\DhStreamParser\StdMp4Parse.h
# End Source File
# Begin Source File

SOURCE=.\decode\DhStreamParser\StreamParser.cpp
# End Source File
# Begin Source File

SOURCE=.\decode\DhStreamParser\StreamParser.h
# End Source File
# Begin Source File

SOURCE=.\decode\DhStreamParser\tsstream.cpp
# End Source File
# Begin Source File

SOURCE=.\decode\DhStreamParser\tsstream.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\decode\audiodecode.cpp
# End Source File
# Begin Source File

SOURCE=.\decode\audiodecode.h
# End Source File
# Begin Source File

SOURCE=.\decode\decode.h
# End Source File
# Begin Source File

SOURCE=.\decode\FifoBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\decode\FifoBuffer.h
# End Source File
# Begin Source File

SOURCE=.\decode\graphfactory.cpp
# End Source File
# Begin Source File

SOURCE=.\decode\graphfactory.h
# End Source File
# Begin Source File

SOURCE=.\decode\h264videodecode.cpp
# End Source File
# Begin Source File

SOURCE=.\decode\h264videodecode.h
# End Source File
# Begin Source File

SOURCE=.\decode\interface.h
# End Source File
# Begin Source File

SOURCE=.\decode\mp4decoder.cpp
# End Source File
# Begin Source File

SOURCE=.\decode\mp4decoder.h
# End Source File
# Begin Source File

SOURCE=.\decode\mpegaudiodecoder.cpp
# End Source File
# Begin Source File

SOURCE=.\decode\mpegaudiodecoder.h
# End Source File
# Begin Source File

SOURCE=.\decode\playgraph.cpp
# End Source File
# Begin Source File

SOURCE=.\decode\playgraph.h
# End Source File
# Begin Source File

SOURCE=.\decode\videorender.cpp
# End Source File
# Begin Source File

SOURCE=.\decode\videorender.h
# End Source File
# End Group
# Begin Group "render"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\render\audiorender.h
# End Source File
# Begin Source File

SOURCE=.\render\colorspace.h
# End Source File
# Begin Source File

SOURCE=.\render\ddoffscreenrender.cpp
# End Source File
# Begin Source File

SOURCE=.\render\ddoffscreenrender.h
# End Source File
# Begin Source File

SOURCE=.\render\ddoverlayrender.cpp
# End Source File
# Begin Source File

SOURCE=.\render\ddoverlayrender.h
# End Source File
# Begin Source File

SOURCE=.\render\gdirender.cpp
# End Source File
# Begin Source File

SOURCE=.\render\gdirender.h
# End Source File
# Begin Source File

SOURCE=.\render\manager.cpp
# End Source File
# Begin Source File

SOURCE=.\render\manager.h
# End Source File
# Begin Source File

SOURCE=.\render\MonitorManager.cpp
# End Source File
# Begin Source File

SOURCE=.\render\MonitorManager.h
# End Source File
# Begin Source File

SOURCE=.\render\pcmrender.cpp
# End Source File
# Begin Source File

SOURCE=.\render\pcmrender.h
# End Source File
# Begin Source File

SOURCE=.\render\videorender.h
# End Source File
# End Group
# Begin Group "depend"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\depend\amr_dec.h
# End Source File
# Begin Source File

SOURCE=.\depend\amr_enc.h
# End Source File
# Begin Source File

SOURCE=.\depend\DllDeinterlace.h
# End Source File
# Begin Source File

SOURCE=.\depend\hi_voice_api.h
# End Source File
# Begin Source File

SOURCE=.\depend\LumaStretch.h
# End Source File
# Begin Source File

SOURCE=.\depend\xvid.h
# End Source File
# Begin Source File

SOURCE=.\depend\libirc.lib
# End Source File
# Begin Source File

SOURCE=.\depend\libm.lib
# End Source File
# Begin Source File

SOURCE=.\depend\DEINTLIB.lib
# End Source File
# Begin Source File

SOURCE=.\depend\ADPCM.lib
# End Source File
# Begin Source File

SOURCE=.\depend\libmad.lib
# End Source File
# Begin Source File

SOURCE=.\depend\hisi_voice_engine.lib
# End Source File
# Begin Source File

SOURCE=.\depend\DllDeinterlace.lib
# End Source File
# Begin Source File

SOURCE=.\depend\AmrDll.lib
# End Source File
# Begin Source File

SOURCE=.\depend\LumaStretch.lib
# End Source File
# Begin Source File

SOURCE=.\depend\g723_lib.lib
# End Source File
# Begin Source File

SOURCE=.\depend\libh264.lib
# End Source File
# Begin Source File

SOURCE=.\depend\core.lib
# End Source File
# End Group
# End Target
# End Project
