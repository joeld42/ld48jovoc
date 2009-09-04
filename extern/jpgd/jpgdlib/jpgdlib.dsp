# Microsoft Developer Studio Project File - Name="jpgdlib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=jpgdlib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "jpgdlib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "jpgdlib.mak" CFG="jpgdlib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "jpgdlib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "jpgdlib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl6.exe
RSC=rc.exe

!IF  "$(CFG)" == "jpgdlib - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /W3 /GX /O2 /Ob2 /I "..\inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_USE_NON_INTEL_COMPILER" /YX /FD /c
# SUBTRACT CPP /nologo /Z<none>
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=xilink6.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\jpgdlib.lib"

!ELSEIF  "$(CFG)" == "jpgdlib - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /W3 /Gm /GX /ZI /Od /I "..\inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_USE_NON_INTEL_COMPILER" /YX /FD /GZ /c
# SUBTRACT CPP /nologo
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=xilink6.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\jpgdlib_d.lib"

!ENDIF 

# Begin Target

# Name "jpgdlib - Win32 Release"
# Name "jpgdlib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\h2v2.cpp

!IF  "$(CFG)" == "jpgdlib - Win32 Release"

# SUBTRACT CPP /D "_USE_NON_INTEL_COMPILER"

!ELSEIF  "$(CFG)" == "jpgdlib - Win32 Debug"

# ADD CPP /ZI
# SUBTRACT CPP /D "_USE_NON_INTEL_COMPILER"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\idct.cpp
# End Source File
# Begin Source File

SOURCE=.\jidctfst.cpp
# End Source File
# Begin Source File

SOURCE=.\jpegdecoder.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\inc\jpegdecoder.h
# End Source File
# Begin Source File

SOURCE=..\inc\jpegdecoder.inl
# End Source File
# Begin Source File

SOURCE=..\inc\main.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\readme.txt
# End Source File
# End Target
# End Project
