# Microsoft Developer Studio Project File - Name="dvr" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=dvr - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "dvr.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dvr.mak" CFG="dvr - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dvr - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "dvr - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "dvr - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DVR_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DVR_EXPORTS" /YX /FD /c
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
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /pdb:none /machine:I386 /out:"../../Bin/Release/dvr.dll"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "dvr - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DVR_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DVR_EXPORTS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"../../Bin/Debug/dvr.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "dvr - Win32 Release"
# Name "dvr - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Des.cpp
# End Source File
# Begin Source File

SOURCE=.\devprob.cpp
# End Source File
# Begin Source File

SOURCE=.\dvr.cpp
# End Source File
# Begin Source File

SOURCE=.\dvr.rc
# End Source File
# Begin Source File

SOURCE=.\dvrinterface.cpp
# End Source File
# Begin Source File

SOURCE=.\osIndependent.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\kernel\afkdef.h
# End Source File
# Begin Source File

SOURCE=.\kernel\afkinc.h
# End Source File
# Begin Source File

SOURCE=.\kernel\afkplugin.h
# End Source File
# Begin Source File

SOURCE=.\Des.h
# End Source File
# Begin Source File

SOURCE=.\def.h
# End Source File
# Begin Source File

SOURCE=.\devprob.h
# End Source File
# Begin Source File

SOURCE=.\dvr.h
# End Source File
# Begin Source File

SOURCE=.\dvrdevice\dvr2cfg.h
# End Source File
# Begin Source File

SOURCE=.\dvrinterface.h
# End Source File
# Begin Source File

SOURCE=.\osIndependent.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "net"

# PROP Default_Filter "网络类"
# Begin Source File

SOURCE=.\Net\Broadcast.cpp
# End Source File
# Begin Source File

SOURCE=.\Net\Broadcast.h
# End Source File
# Begin Source File

SOURCE=.\Net\MulticastSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\Net\MulticastSocket.h
# End Source File
# Begin Source File

SOURCE=.\Net\SocketCallBack.cpp
# End Source File
# Begin Source File

SOURCE=.\Net\SocketCallBack.h
# End Source File
# Begin Source File

SOURCE=.\Net\TcpSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\Net\TcpSocket.h
# End Source File
# Begin Source File

SOURCE=.\Net\TcpSockServer.cpp
# End Source File
# Begin Source File

SOURCE=.\Net\TcpSockServer.h
# End Source File
# Begin Source File

SOURCE=.\Net\UdpSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\Net\UdpSocket.h
# End Source File
# End Group
# Begin Group "dvrdevice"

# PROP Default_Filter "设备类"
# Begin Group "mediachannel"

# PROP Default_Filter "实时监视通道"
# Begin Source File

SOURCE=.\dvrdevice\dvrmediachannel.cpp
# End Source File
# Begin Source File

SOURCE=.\dvrdevice\dvrmediachannel.h
# End Source File
# End Group
# Begin Group "packet"

# PROP Default_Filter "网络包"
# Begin Source File

SOURCE=.\dvrdevice\dvrpacket_comm.cpp
# End Source File
# Begin Source File

SOURCE=.\dvrdevice\dvrpacket_comm.h
# End Source File
# Begin Source File

SOURCE=.\dvrdevice\dvrpacket_DDNS.cpp
# End Source File
# Begin Source File

SOURCE=.\dvrdevice\dvrpacket_DDNS.h
# End Source File
# Begin Source File

SOURCE=.\dvrdevice\dvrpacket_dvr2.cpp
# End Source File
# Begin Source File

SOURCE=.\dvrdevice\dvrpacket_dvr2.h
# End Source File
# Begin Source File

SOURCE=.\dvrdevice\dvrpacket_mobile.cpp
# End Source File
# Begin Source File

SOURCE=.\dvrdevice\dvrpacket_mobile.h
# End Source File
# Begin Source File

SOURCE=.\ParseString.cpp
# End Source File
# Begin Source File

SOURCE=.\ParseString.h
# End Source File
# End Group
# Begin Group "searchchannel"

# PROP Default_Filter "查询通道"
# Begin Source File

SOURCE=.\dvrdevice\dvrsearchchannel.cpp
# End Source File
# Begin Source File

SOURCE=.\dvrdevice\dvrsearchchannel.h
# End Source File
# Begin Source File

SOURCE=.\dvrdevice\dvrsearchchannel_DDNS.cpp
# End Source File
# Begin Source File

SOURCE=.\dvrdevice\dvrsearchchannel_DDNS.h
# End Source File
# End Group
# Begin Group "downloadchannel"

# PROP Default_Filter "下载通道"
# Begin Source File

SOURCE=.\dvrdevice\dvrdownloadchannel.cpp
# End Source File
# Begin Source File

SOURCE=.\dvrdevice\dvrdownloadchannel.h
# End Source File
# End Group
# Begin Group "upgradechannel"

# PROP Default_Filter "升级通道"
# Begin Source File

SOURCE=.\dvrdevice\dvrupgradechannel.cpp
# End Source File
# Begin Source File

SOURCE=.\dvrdevice\dvrupgradechannel.h
# End Source File
# End Group
# Begin Group "previewchannel"

# PROP Default_Filter "预览通道"
# Begin Source File

SOURCE=.\dvrdevice\dvrpreviewchannel.cpp
# End Source File
# Begin Source File

SOURCE=.\dvrdevice\dvrpreviewchannel.h
# End Source File
# End Group
# Begin Group "device"

# PROP Default_Filter "设备"
# Begin Source File

SOURCE=.\dvrdevice\DvrDevice_DDNS.cpp
# End Source File
# Begin Source File

SOURCE=.\dvrdevice\DvrDevice_DDNS.h
# End Source File
# End Group
# Begin Group "controlchannel"

# PROP Default_Filter "控制通道"
# Begin Source File

SOURCE=.\dvrdevice\dvrcontrolchannel.cpp
# End Source File
# Begin Source File

SOURCE=.\dvrdevice\dvrcontrolchannel.h
# End Source File
# End Group
# Begin Group "configchannel"

# PROP Default_Filter "配置通道"
# Begin Source File

SOURCE=.\dvrdevice\dvrconfigchannel.cpp
# End Source File
# Begin Source File

SOURCE=.\dvrdevice\dvrconfigchannel.h
# End Source File
# End Group
# Begin Group "statiscchannel"

# PROP Default_Filter "统计通道"
# Begin Source File

SOURCE=.\dvrdevice\dvrstatiscchannel.cpp
# End Source File
# Begin Source File

SOURCE=.\dvrdevice\dvrstatiscchannel.h
# End Source File
# End Group
# Begin Group "talkchannel"

# PROP Default_Filter "对讲通道"
# Begin Source File

SOURCE=.\dvrdevice\dvrtalkchannel.cpp
# End Source File
# Begin Source File

SOURCE=.\dvrdevice\dvrtalkchannel.h
# End Source File
# End Group
# Begin Group "userchannel"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dvrdevice\dvruserchannel.cpp
# End Source File
# Begin Source File

SOURCE=.\dvrdevice\dvruserchannel.h
# End Source File
# End Group
# Begin Group "transchannel"

# PROP Default_Filter "透明通道"
# Begin Source File

SOURCE=.\dvrdevice\dvrtranschannel.cpp
# End Source File
# Begin Source File

SOURCE=.\dvrdevice\dvrtranschannel.h
# End Source File
# End Group
# Begin Group "alarmchannel"

# PROP Default_Filter "报警通道"
# Begin Source File

SOURCE=.\dvrdevice\dvralarmchannel.cpp
# End Source File
# Begin Source File

SOURCE=.\dvrdevice\dvralarmchannel.h
# End Source File
# End Group
# Begin Group "capturechannel"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dvrdevice\dvrsnapchannel.cpp
# End Source File
# Begin Source File

SOURCE=.\dvrdevice\dvrsnapchannel.h
# End Source File
# End Group
# Begin Group "GpsChannel"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dvrdevice\DvrGpsChannel.cpp
# End Source File
# Begin Source File

SOURCE=.\dvrdevice\DvrGpsChannel.h
# End Source File
# End Group
# Begin Group "RequestChannel"

# PROP Default_Filter "请求通道，异步接口的操作包"
# Begin Source File

SOURCE=.\dvrdevice\RequestChannel.cpp
# End Source File
# Begin Source File

SOURCE=.\dvrdevice\RequestChannel.h
# End Source File
# End Group
# Begin Group "newcofigchannel"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dvrdevice\DvrNewConfigChannel.cpp
# End Source File
# Begin Source File

SOURCE=.\dvrdevice\DvrNewConfigChannel.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\dvrcap.cpp
# End Source File
# Begin Source File

SOURCE=.\dvrcap.h
# End Source File
# Begin Source File

SOURCE=.\dvrdevice\dvrchannel.cpp
# End Source File
# Begin Source File

SOURCE=.\dvrdevice\dvrchannel.h
# End Source File
# Begin Source File

SOURCE=.\dvrdevice\dvrdevice.cpp
# End Source File
# Begin Source File

SOURCE=.\dvrdevice\dvrdevice.h
# End Source File
# End Group
# End Target
# End Project
