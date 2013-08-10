# Microsoft Developer Studio Project File - Name="cDolphin_lib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=cDolphin_lib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "cDolphin_lib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "cDolphin_lib.mak" CFG="cDolphin_lib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "cDolphin_lib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "cDolphin_lib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl6.exe
RSC=rc.exe

!IF  "$(CFG)" == "cDolphin_lib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\..\tmp\windows\x86\vc60\Release\cDolphin_lib"
# PROP Intermediate_Dir "..\..\..\..\tmp\windows\x86\vc60\Release\cDolphin_lib"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\..\..\..\include" /I "..\..\..\..\include\cDolphin" /I "..\..\..\..\src" /I "..\..\..\..\src\cDolphin" /I "..\..\..\..\deps" /I "..\..\..\..\lib\windows\x86\vc60" /D "WIN32" /D "NDEBUG" /D "_LIB" /D "_WINDOWS" /D "_MBCS" /D "ZLIB_WINAPI" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_WARNINGS" /FD /O3 /G7 /QxP /Qipo /Qprec-div- /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=xilink6.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\..\lib\windows\x86\vc60\cDolphin_lib.lib"

!ELSEIF  "$(CFG)" == "cDolphin_lib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\..\tmp\windows\x86\vc60\Debug\cDolphin_lib"
# PROP Intermediate_Dir "..\..\..\..\tmp\windows\x86\vc60\Debug\cDolphin_lib"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\..\..\include" /I "..\..\..\..\include\cDolphin" /I "..\..\..\..\src" /I "..\..\..\..\src\cDolphin" /I "..\..\..\..\deps" /I "..\..\..\..\lib\windows\x86\vc60" /D "WIN32" /D "_DEBUG" /D "_LIB" /D "_WINDOWS" /D "_MBCS" /D "ZLIB_WINAPI" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_WARNINGS" /FD /GZ /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=xilink6.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\..\lib\windows\x86\vc60\cDolphin_lib_debug.lib"

!ENDIF 

# Begin Target

# Name "cDolphin_lib - Win32 Release"
# Name "cDolphin_lib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\autoplay.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\bitbchk.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\bitbcnt.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\bitbcnt2.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\bitbmob.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\bitboard.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\bitbtest.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\bitbtest1.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\bitbtest2.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\bitbtest4.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\bitbtest5.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\bitbtest6.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\bitbvald.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\board.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\coeffs.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\counter.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\currency.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\display.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\endgame.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\epcstat.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\error.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\eval.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\game.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\global.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\hash.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\learn.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\midgame.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\move.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\myrandom.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\opname.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\osfbook.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\pattern.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\pattern_tainer.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\pcstat.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\probcut.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\pv.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\safemem.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\search.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\stable.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\timer.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\cDolphin\utils.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\autoplay.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\bitbchk.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\bitbcnt.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\bitbcnt2.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\bitbmob.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\bitboard.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\bitbtest.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\bitbtest1.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\bitbtest2.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\bitbtest4.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\bitbtest5.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\bitbtest6.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\bitbvald.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\board.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\coeffs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\colour.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\counter.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\currency.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\display.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\dolphin.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\endgame.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\epcstat.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\error.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\eval.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\game.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\global.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\hash.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\learn.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\magic.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\midgame.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\move.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\myrandom.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\opname.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\osfbook.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\pattern.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\pattern_tainer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\pcstat.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\probcut.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\pv.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\safemem.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\search.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\stable.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\texts.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\timer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\cDolphin\utils.h
# End Source File
# End Group
# End Target
# End Project
