# Microsoft Developer Studio Project File - Name="zlib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=zlib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "zlib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "zlib.mak" CFG="zlib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "zlib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "zlib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl6.exe
RSC=rc.exe

!IF  "$(CFG)" == "zlib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\..\tmp\windows\x86\vc60\Release\zlib"
# PROP Intermediate_Dir "..\..\..\..\tmp\windows\x86\vc60\Release\zlib"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\..\..\..\deps\zlib" /I "..\..\..\..\deps\zlib\contrib\masmx86" /D "WIN32" /D "NDEBUG" /D "_LIB" /D "_WINDOWS" /D "_MBCS" /D "ZLIB_WINAPI" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_WARNINGS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=xilink6.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\..\lib\windows\x86\vc60\zlib.lib"

!ELSEIF  "$(CFG)" == "zlib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\..\tmp\windows\x86\vc60\Debug\zlib"
# PROP Intermediate_Dir "..\..\..\..\tmp\windows\x86\vc60\Debug\zlib"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\..\..\deps\zlib" /I "..\..\..\..\deps\zlib\contrib\masmx86" /D "WIN32" /D "_DEBUG" /D "_LIB" /D "_WINDOWS" /D "_MBCS" /D "ZLIB_WINAPI" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_WARNINGS" /FD /GZ /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=xilink6.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\..\lib\windows\x86\vc60\zlib_debug.lib"

!ENDIF 

# Begin Target

# Name "zlib - Win32 Release"
# Name "zlib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\..\deps\zlib\adler32.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\deps\zlib\compress.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\deps\zlib\crc32.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\deps\zlib\deflate.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\deps\zlib\gzclose.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\deps\zlib\gzlib.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\deps\zlib\gzread.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\deps\zlib\gzwrite.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\deps\zlib\infback.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\deps\zlib\inffast.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\deps\zlib\inflate.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\deps\zlib\inftrees.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\deps\zlib\contrib\minizip\ioapi.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\deps\zlib\trees.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\deps\zlib\uncompr.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\deps\zlib\contrib\vstudio\vc9\zlibvc.def
# End Source File
# Begin Source File

SOURCE=..\..\..\..\deps\zlib\zutil.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\..\deps\zlib\crc32.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\deps\zlib\deflate.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\deps\zlib\gzguts.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\deps\zlib\inffast.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\deps\zlib\inffixed.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\deps\zlib\inflate.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\deps\zlib\inftrees.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\deps\zlib\infutil.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\deps\zlib\contrib\minizip\ioapi.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\deps\zlib\contrib\minizip\iowin32.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\deps\zlib\trees.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\deps\zlib\zconf.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\deps\zlib\zlib.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\deps\zlib\zutil.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\..\..\deps\zlib\win32\zlib1.rc
# End Source File
# End Group
# End Target
# End Project
