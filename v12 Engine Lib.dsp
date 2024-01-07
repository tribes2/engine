# Microsoft Developer Studio Project File - Name="v12 Engine Lib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=v12 Engine Lib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "v12 Engine Lib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "v12 Engine Lib.mak" CFG="v12 Engine Lib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "v12 Engine Lib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "v12 Engine Lib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "out.VC6.RELEASE"
# PROP Intermediate_Dir "out.VC6.RELEASE"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /GR /GX /O2 /I "../lib/directx" /I "../lib/zlib" /I "../lib/lungif" /I "../lib/lpng" /I "../lib/ljpeg" /I "../lib/openal/win32" /I "." /D "WIN32" /D "USEASSEMBLYTERRBLEND" /D "PNG_NO_READ_TIME" /D "PNG_NO_WRITE_TIME" /D OPENGL2D3D=\"glFOO.dll\" /D GLU2D3D=\"gluFOO.dll\" /D "NO_MILES_OPENAL" /YX /FD /c /Tp
# SUBTRACT CPP /Z<none>
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"out.VC6.RELEASE\engine_RELEASE.lib" /NODEFAULTLIB:LIBCD

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "out.VC6.DEBUG"
# PROP Intermediate_Dir "out.VC6.DEBUG"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /Gm /GR /GX /ZI /Od /I "../lib/directx" /I "../lib/zlib" /I "../lib/lungif" /I "../lib/lpng" /I "../lib/ljpeg" /I "../lib/openal/win32" /I "." /D "DEBUG" /D "V12_DEBUG" /D "ENABLE_ASSERTS" /D "WIN32" /D "USEASSEMBLYTERRBLEND" /D "PNG_NO_READ_TIME" /D "PNG_NO_WRITE_TIME" /D "NO_MILES_OPENAL" /D OPENGL2D3D=\"glFOO.dll\" /D GLU2D3D=\"gluFOO.dll\" /YX /FD /GZ /c /Tp
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"out.VC6.DEBUG\engine_DEBUG.lib" /NODEFAULTLIB:LIBCD

!ENDIF 

# Begin Target

# Name "v12 Engine Lib - Win32 Release"
# Name "v12 Engine Lib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "ai"

# PROP Default_Filter "cc"
# End Group
# Begin Group "audio"

# PROP Default_Filter "cc"
# End Group
# Begin Group "collision"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\collision\abstractPolyList.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/collision"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/collision"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\collision\boxConvex.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/collision"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/collision"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\collision\clippedPolyList.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/collision"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/collision"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\collision\convex.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/collision"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/collision"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\collision\depthSortList.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/collision"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/collision"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\collision\earlyOutPolyList.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/collision"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/collision"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\collision\extrudedPolyList.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/collision"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/collision"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\collision\gjk.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/collision"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/collision"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\collision\planeExtractor.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/collision"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/collision"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\collision\polyhedron.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/collision"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/collision"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\collision\polytope.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/collision"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/collision"

!ENDIF 

# End Source File
# End Group
# Begin Group "console"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\console\compiledEval.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/console"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/console"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\console\compiler.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/console"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/console"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\console\console.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/console"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/console"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\console\consoleFunctions.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/console"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/console"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\console\consoleInternal.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/console"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/console"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\console\consoleObject.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/console"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/console"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\console\consoleTypes.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/console"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/console"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\console\gram.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/console"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/console"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\console\scan.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/console"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/console"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\console\scriptObject.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/console"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/console"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\console\simBase.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/console"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/console"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\console\simDictionary.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/console"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/console"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\console\simManager.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/console"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/console"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\console\telnetConsole.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/console"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/console"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\console\telnetDebugger.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/console"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/console"

!ENDIF 

# End Source File
# End Group
# Begin Group "core"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\core\bitRender.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\bitStream.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\BitTables.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\dataChunker.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\dnet.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\fileObject.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\fileStream.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\filterStream.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\findMatch.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\idGenerator.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\memStream.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\nStream.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\nTypes.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\resDictionary.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\resizeStream.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\resManager.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\stringTable.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\tagDictionary.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\tVector.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\zipAggregate.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\zipHeaders.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\zipSubStream.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# End Group
# Begin Group "crypt"

# PROP Default_Filter "cc"
# End Group
# Begin Group "dgl"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\dgl\bitmapBM8.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/dgl"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/dgl"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dgl\bitmapBMP.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/dgl"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/dgl"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dgl\bitmapGIF.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/dgl"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/dgl"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dgl\bitmapJpeg.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/dgl"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/dgl"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dgl\bitmapPng.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/dgl"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/dgl"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dgl\dgl.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/dgl"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/dgl"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dgl\dglMatrix.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/dgl"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/dgl"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dgl\gBitmap.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/dgl"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/dgl"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dgl\gFont.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/dgl"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/dgl"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dgl\gPalette.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/dgl"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/dgl"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dgl\gTexManager.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/dgl"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/dgl"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dgl\lensFlare.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/dgl"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/dgl"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dgl\materialList.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/dgl"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/dgl"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dgl\materialPropertyMap.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/dgl"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/dgl"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dgl\rectClipper.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/dgl"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/dgl"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dgl\splineUtil.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/dgl"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/dgl"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dgl\stripCache.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/dgl"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/dgl"

!ENDIF 

# End Source File
# End Group
# Begin Group "editor"

# PROP Default_Filter "cc"
# End Group
# Begin Group "game"

# PROP Default_Filter "cc"
# End Group
# Begin Group "gui"

# PROP Default_Filter "cc"
# End Group
# Begin Group "hud"

# PROP Default_Filter "cc"
# End Group
# Begin Group "interior"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\interior\FloorPlanRes.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/interior"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/interior"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\interior\forceField.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/interior"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/interior"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\interior\interior.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/interior"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/interior"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\interior\interiorCollision.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/interior"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/interior"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\interior\interiorDebug.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/interior"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/interior"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\interior\interiorInstance.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/interior"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/interior"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\interior\interiorIO.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/interior"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/interior"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\interior\interiorLightAnim.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/interior"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/interior"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\interior\interiorLMManager.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/interior"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/interior"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\interior\interiorRender.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/interior"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/interior"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\interior\interiorRes.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/interior"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/interior"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\interior\interiorResObjects.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/interior"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/interior"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\interior\interiorSubObject.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/interior"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/interior"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\interior\itfdump.asm

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/interior"
# Begin Custom Build
IntDir=.\out.VC6.RELEASE/interior
InputPath=.\interior\itfdump.asm
InputName=itfdump

"$(IntDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw.exe -f win32 $(InputPath) -o $(IntDir)/$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/interior"
# Begin Custom Build
IntDir=.\out.VC6.DEBUG/interior
InputPath=.\interior\itfdump.asm
InputName=itfdump

"$(IntDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw.exe -f win32 $(InputPath) -o $(IntDir)/$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\interior\lightUpdateGrouper.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/interior"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/interior"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\interior\mirrorSubObject.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/interior"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/interior"

!ENDIF 

# End Source File
# End Group
# Begin Group "math"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\math\mathTypes.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/math"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/math"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\math\mathUtils.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/math"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/math"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\math\mBox.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/math"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/math"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\math\mConsoleFunctions.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/math"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/math"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\math\mMath_C.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/math"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/math"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\math\mMathAMD.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/math"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/math"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\math\mMathFn.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/math"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/math"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\math\mMathSSE.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/math"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/math"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\math\mMatrix.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/math"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/math"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\math\mPlaneTransformer.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/math"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/math"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\math\mQuadPatch.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/math"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/math"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\math\mQuat.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/math"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/math"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\math\mRandom.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/math"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/math"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\math\mSolver.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/math"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/math"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\math\mSplinePatch.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/math"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/math"

!ENDIF 

# End Source File
# End Group
# Begin Group "platform"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\platform\gameInterface.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platform"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platform"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platform\platformAssert.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platform"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platform"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platform\platformMemory.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platform"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platform"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platform\platformRedBook.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platform"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platform"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platform\platformVideo.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platform"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platform"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platform\profiler.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platform"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platform"

!ENDIF 

# End Source File
# End Group
# Begin Group "platformWIN32"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\platformWIN32\winAsmBlit.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winConsole.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winCPUInfo.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winD3DVideo.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winDInputDevice.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winDirectInput.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winFileio.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winFont.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winGL.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winInput.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winMath.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winMath_ASM.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winMemory.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winMutex.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winNet.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winOGLVideo.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winOpenAL.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winProcessControl.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winRedbook.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winSemaphore.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winStrings.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winThread.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winTime.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winV2Video.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winWindow.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# End Group
# Begin Group "sceneGraph"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\scenegraph\detailManager.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sceneGraph"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sceneGraph"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\scenegraph\lightManager.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sceneGraph"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sceneGraph"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\scenegraph\sceneGraph.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sceneGraph"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sceneGraph"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\scenegraph\sceneLighting.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sceneGraph"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sceneGraph"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\scenegraph\sceneRoot.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sceneGraph"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sceneGraph"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\scenegraph\sceneState.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sceneGraph"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sceneGraph"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\scenegraph\sceneTraversal.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sceneGraph"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sceneGraph"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\scenegraph\sgUtil.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sceneGraph"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sceneGraph"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\scenegraph\shadowVolumeBSP.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sceneGraph"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sceneGraph"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\scenegraph\windingClipper.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sceneGraph"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sceneGraph"

!ENDIF 

# End Source File
# End Group
# Begin Group "shell"

# PROP Default_Filter "cc"
# End Group
# Begin Group "sim"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\sim\actionMap.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sim"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sim"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\cannedChatDataBlock.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sim"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sim"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\decalManager.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sim"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sim"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\frameAllocator.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sim"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sim"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\netConnection.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sim"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sim"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\netEvent.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sim"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sim"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\netGhost.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sim"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sim"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\netObject.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sim"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sim"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\netStringTable.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sim"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sim"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\pathManager.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sim"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sim"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\sceneObject.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sim"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sim"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\simPath.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sim"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sim"

!ENDIF 

# End Source File
# End Group
# Begin Group "terrain"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\terrain\blender.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/terrain"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/terrain"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\terrain\bvQuadTree.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/terrain"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/terrain"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\terrain\FluidQuadTree.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/terrain"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/terrain"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\terrain\FluidRender.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/terrain"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/terrain"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\terrain\FluidSupport.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/terrain"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/terrain"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\terrain\Sky.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/terrain"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/terrain"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\terrain\Sun.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/terrain"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/terrain"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\terrain\terrCollision.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/terrain"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/terrain"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\terrain\terrData.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/terrain"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/terrain"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\terrain\terrLighting.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/terrain"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/terrain"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\terrain\terrRender.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/terrain"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/terrain"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\terrain\terrRender2.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/terrain"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/terrain"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\terrain\waterBlock.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/terrain"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/terrain"

!ENDIF 

# End Source File
# End Group
# Begin Group "ts"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\ts\tsAnimate.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ts"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ts"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ts\tsCollision.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ts"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ts"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ts\tsDecal.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ts"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ts"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ts\tsDump.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ts"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ts"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ts\tsIntegerSet.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ts"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ts"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ts\tsLastDetail.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ts"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ts"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ts\tsMaterialList.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ts"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ts"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ts\tsMesh.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ts"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ts"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ts\tsPartInstance.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ts"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ts"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ts\tsShape.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ts"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ts"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ts\tsShapeAlloc.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ts"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ts"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ts\tsShapeConstruct.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ts"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ts"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ts\tsShapeInstance.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ts"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ts"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ts\tsShapeOldRead.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ts"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ts"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ts\tsSortedMesh.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ts"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ts"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ts\tsThread.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ts"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ts"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ts\tsTransform.cc

!IF  "$(CFG)" == "v12 Engine Lib - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ts"

!ELSEIF  "$(CFG)" == "v12 Engine Lib - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ts"

!ENDIF 

# End Source File
# End Group
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "ai headers"

# PROP Default_Filter "h"
# End Group
# Begin Group "audio headers"

# PROP Default_Filter "h"
# End Group
# Begin Group "collision headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\collision\abstractPolyList.h
# End Source File
# Begin Source File

SOURCE=.\collision\boxConvex.h
# End Source File
# Begin Source File

SOURCE=.\collision\clippedPolyList.h
# End Source File
# Begin Source File

SOURCE=.\collision\collision.h
# End Source File
# Begin Source File

SOURCE=.\collision\convex.h
# End Source File
# Begin Source File

SOURCE=.\collision\depthSortList.h
# End Source File
# Begin Source File

SOURCE=.\collision\earlyOutPolyList.h
# End Source File
# Begin Source File

SOURCE=.\collision\extrudedPolyList.h
# End Source File
# Begin Source File

SOURCE=.\collision\gjk.h
# End Source File
# Begin Source File

SOURCE=.\collision\planeExtractor.h
# End Source File
# Begin Source File

SOURCE=.\collision\polyhedron.h
# End Source File
# Begin Source File

SOURCE=.\collision\polytope.h
# End Source File
# End Group
# Begin Group "console headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\console\ast.h
# End Source File
# Begin Source File

SOURCE=.\console\compiler.h
# End Source File
# Begin Source File

SOURCE=.\console\console.h
# End Source File
# Begin Source File

SOURCE=.\console\consoleInternal.h
# End Source File
# Begin Source File

SOURCE=.\console\consoleObject.h
# End Source File
# Begin Source File

SOURCE=.\console\consoleTypes.h
# End Source File
# Begin Source File

SOURCE=.\console\gram.h
# End Source File
# Begin Source File

SOURCE=.\console\objectTypes.h
# End Source File
# Begin Source File

SOURCE=.\console\simBase.h
# End Source File
# Begin Source File

SOURCE=.\console\simDictionary.h
# End Source File
# Begin Source File

SOURCE=.\console\telnetConsole.h
# End Source File
# Begin Source File

SOURCE=.\console\telnetDebugger.h
# End Source File
# End Group
# Begin Group "core headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\core\bitMatrix.h
# End Source File
# Begin Source File

SOURCE=.\core\bitRender.h
# End Source File
# Begin Source File

SOURCE=.\core\bitSet.h
# End Source File
# Begin Source File

SOURCE=.\core\bitStream.h
# End Source File
# Begin Source File

SOURCE=.\core\BitTables.h
# End Source File
# Begin Source File

SOURCE=.\core\bitVector.h
# End Source File
# Begin Source File

SOURCE=.\core\bitVectorW.h
# End Source File
# Begin Source File

SOURCE=.\core\color.h
# End Source File
# Begin Source File

SOURCE=.\core\coreRes.h
# End Source File
# Begin Source File

SOURCE=.\core\dataChunker.h
# End Source File
# Begin Source File

SOURCE=.\core\dnet.h
# End Source File
# Begin Source File

SOURCE=.\core\fileio.h
# End Source File
# Begin Source File

SOURCE=.\core\fileObject.h
# End Source File
# Begin Source File

SOURCE=.\core\fileStream.h
# End Source File
# Begin Source File

SOURCE=.\core\filterStream.h
# End Source File
# Begin Source File

SOURCE=.\core\findMatch.h
# End Source File
# Begin Source File

SOURCE=.\core\idGenerator.h
# End Source File
# Begin Source File

SOURCE=.\core\llist.h
# End Source File
# Begin Source File

SOURCE=.\core\memstream.h
# End Source File
# Begin Source File

SOURCE=.\core\polyList.h
# End Source File
# Begin Source File

SOURCE=.\core\realComp.h
# End Source File
# Begin Source File

SOURCE=.\core\resizeStream.h
# End Source File
# Begin Source File

SOURCE=.\core\resManager.h
# End Source File
# Begin Source File

SOURCE=.\core\stream.h
# End Source File
# Begin Source File

SOURCE=.\core\stringTable.h
# End Source File
# Begin Source File

SOURCE=.\core\tagDictionary.h
# End Source File
# Begin Source File

SOURCE=.\core\tAlgorithm.h
# End Source File
# Begin Source File

SOURCE=.\core\tSparseArray.h
# End Source File
# Begin Source File

SOURCE=.\core\tVector.h
# End Source File
# Begin Source File

SOURCE=.\core\zipAggregate.h
# End Source File
# Begin Source File

SOURCE=.\core\zipHeaders.h
# End Source File
# Begin Source File

SOURCE=.\core\zipSubStream.h
# End Source File
# End Group
# Begin Group "crypt headers"

# PROP Default_Filter "h"
# End Group
# Begin Group "dgl headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\dgl\dgl.h
# End Source File
# Begin Source File

SOURCE=.\dgl\gBitmap.h
# End Source File
# Begin Source File

SOURCE=.\dgl\gChunkedTexManager.h
# End Source File
# Begin Source File

SOURCE=.\dgl\gFont.h
# End Source File
# Begin Source File

SOURCE=.\dgl\gPalette.h
# End Source File
# Begin Source File

SOURCE=.\dgl\gTexManager.h
# End Source File
# Begin Source File

SOURCE=.\dgl\lensFlare.h
# End Source File
# Begin Source File

SOURCE=.\dgl\materialList.h
# End Source File
# Begin Source File

SOURCE=.\dgl\materialPropertyMap.h
# End Source File
# Begin Source File

SOURCE=.\dgl\rectClipper.h
# End Source File
# Begin Source File

SOURCE=.\dgl\splineUtil.h
# End Source File
# Begin Source File

SOURCE=.\dgl\stripCache.h
# End Source File
# End Group
# Begin Group "editor headers"

# PROP Default_Filter "h"
# End Group
# Begin Group "game headers"

# PROP Default_Filter "h"
# End Group
# Begin Group "gui headers"

# PROP Default_Filter "h"
# End Group
# Begin Group "hud headers"

# PROP Default_Filter "h"
# End Group
# Begin Group "interior headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\interior\FloorPlanRes.h
# End Source File
# Begin Source File

SOURCE=.\interior\forceField.h
# End Source File
# Begin Source File

SOURCE=.\interior\interior.h
# End Source File
# Begin Source File

SOURCE=.\interior\interiorInstance.h
# End Source File
# Begin Source File

SOURCE=.\interior\interiorLMManager.h
# End Source File
# Begin Source File

SOURCE=.\interior\interiorRes.h
# End Source File
# Begin Source File

SOURCE=.\interior\interiorResObjects.h
# End Source File
# Begin Source File

SOURCE=.\interior\interiorSubObject.h
# End Source File
# Begin Source File

SOURCE=.\interior\itf.h
# End Source File
# Begin Source File

SOURCE=.\interior\lightUpdateGrouper.h
# End Source File
# Begin Source File

SOURCE=.\interior\mirrorSubObject.h
# End Source File
# End Group
# Begin Group "math headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\math\mathIO.h
# End Source File
# Begin Source File

SOURCE=.\math\mathTypes.h
# End Source File
# Begin Source File

SOURCE=.\math\mathUtils.h
# End Source File
# Begin Source File

SOURCE=.\math\mBox.h
# End Source File
# Begin Source File

SOURCE=.\math\mConstants.h
# End Source File
# Begin Source File

SOURCE=.\math\mMath.h
# End Source File
# Begin Source File

SOURCE=.\math\mMathFn.h
# End Source File
# Begin Source File

SOURCE=.\math\mMatrix.h
# End Source File
# Begin Source File

SOURCE=.\math\mPlane.h
# End Source File
# Begin Source File

SOURCE=.\math\mPlaneTransformer.h
# End Source File
# Begin Source File

SOURCE=.\math\mPoint.h
# End Source File
# Begin Source File

SOURCE=.\math\mQuadPatch.h
# End Source File
# Begin Source File

SOURCE=.\math\mQuat.h
# End Source File
# Begin Source File

SOURCE=.\math\mRandom.h
# End Source File
# Begin Source File

SOURCE=.\math\mRect.h
# End Source File
# Begin Source File

SOURCE=.\math\mSphere.h
# End Source File
# Begin Source File

SOURCE=.\math\mSplinePatch.h
# End Source File
# Begin Source File

SOURCE=.\math\mTrig.h
# End Source File
# End Group
# Begin Group "platform headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\platform\3DFX.H
# End Source File
# Begin Source File

SOURCE=.\platform\event.h
# End Source File
# Begin Source File

SOURCE=.\platform\gameInterface.h
# End Source File
# Begin Source File

SOURCE=.\platform\platform.h
# End Source File
# Begin Source File

SOURCE=.\platform\platformAssert.h
# End Source File
# Begin Source File

SOURCE=.\platform\platformAudio.h
# End Source File
# Begin Source File

SOURCE=.\platform\platformInput.h
# End Source File
# Begin Source File

SOURCE=.\platform\platformMutex.h
# End Source File
# Begin Source File

SOURCE=.\platform\platformRedBook.h
# End Source File
# Begin Source File

SOURCE=.\platform\platformSemaphore.h
# End Source File
# Begin Source File

SOURCE=.\platform\platformThread.h
# End Source File
# Begin Source File

SOURCE=.\platform\platformVideo.h
# End Source File
# Begin Source File

SOURCE=.\platform\profiler.h
# End Source File
# Begin Source File

SOURCE=.\platform\types.h
# End Source File
# Begin Source File

SOURCE=.\platform\typesWin32.h
# End Source File
# End Group
# Begin Group "platformWIN32 headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\platformWIN32\gllist.h
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\platformAL.h
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\platformGL.h
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\platformWin32.h
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winConsole.h
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winD3DVideo.h
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winDInputDevice.h
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winDirectInput.h
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winOGLVideo.h
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winV2Video.h
# End Source File
# End Group
# Begin Group "sceneGraph headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\scenegraph\detailManager.h
# End Source File
# Begin Source File

SOURCE=.\scenegraph\lightManager.h
# End Source File
# Begin Source File

SOURCE=.\scenegraph\sceneGraph.h
# End Source File
# Begin Source File

SOURCE=.\scenegraph\sceneLighting.h
# End Source File
# Begin Source File

SOURCE=.\scenegraph\sceneRoot.h
# End Source File
# Begin Source File

SOURCE=.\scenegraph\sceneState.h
# End Source File
# Begin Source File

SOURCE=.\scenegraph\sgUtil.h
# End Source File
# Begin Source File

SOURCE=.\scenegraph\shadowVolumeBSP.h
# End Source File
# Begin Source File

SOURCE=.\scenegraph\windingClipper.h
# End Source File
# End Group
# Begin Group "shell headers"

# PROP Default_Filter "h"
# End Group
# Begin Group "sim headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\sim\actionMap.h
# End Source File
# Begin Source File

SOURCE=.\sim\cannedChatDataBlock.h
# End Source File
# Begin Source File

SOURCE=.\sim\decalManager.h
# End Source File
# Begin Source File

SOURCE=.\sim\frameAllocator.h
# End Source File
# Begin Source File

SOURCE=.\sim\netConnection.h
# End Source File
# Begin Source File

SOURCE=.\sim\netObject.h
# End Source File
# Begin Source File

SOURCE=.\sim\netStringTable.h
# End Source File
# Begin Source File

SOURCE=.\sim\pathManager.h
# End Source File
# Begin Source File

SOURCE=.\sim\sceneObject.h
# End Source File
# Begin Source File

SOURCE=.\sim\simPath.h
# End Source File
# End Group
# Begin Group "terrain headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\terrain\blender.h
# End Source File
# Begin Source File

SOURCE=.\terrain\bvQuadTree.h
# End Source File
# Begin Source File

SOURCE=.\terrain\Fluid.h
# End Source File
# Begin Source File

SOURCE=.\terrain\Sky.h
# End Source File
# Begin Source File

SOURCE=.\terrain\Sun.h
# End Source File
# Begin Source File

SOURCE=.\terrain\terrData.h
# End Source File
# Begin Source File

SOURCE=.\terrain\terrRender.h
# End Source File
# Begin Source File

SOURCE=.\terrain\waterBlock.h
# End Source File
# End Group
# Begin Group "ts headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\ts\tsDecal.h
# End Source File
# Begin Source File

SOURCE=.\ts\tsIntegerSet.h
# End Source File
# Begin Source File

SOURCE=.\ts\tsLastDetail.h
# End Source File
# Begin Source File

SOURCE=.\ts\tsMesh.h
# End Source File
# Begin Source File

SOURCE=.\ts\tsPartInstance.h
# End Source File
# Begin Source File

SOURCE=.\ts\tsShape.h
# End Source File
# Begin Source File

SOURCE=.\ts\tsShapeAlloc.h
# End Source File
# Begin Source File

SOURCE=.\ts\tsShapeConstruct.h
# End Source File
# Begin Source File

SOURCE=.\ts\tsShapeInstance.h
# End Source File
# Begin Source File

SOURCE=.\ts\tsSortedMesh.h
# End Source File
# Begin Source File

SOURCE=.\ts\tsTransform.h
# End Source File
# End Group
# End Group
# End Target
# End Project
